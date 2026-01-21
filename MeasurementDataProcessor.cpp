#include "measurementdataprocessor.h"
#include <QtEndian>
#include <QDebug>

MeasurementDataProcessor::MeasurementDataProcessor(QObject* parent) : QObject(parent) {
    // 必须放在构造函数里，程序运行到这里才会执行注册
    qRegisterMetaType<QVector<quint16>>("QVector<quint16>");
    qRegisterMetaType<QVector<quint32>>("QVector<quint32>");

    m_adcPool.reserve(1000000);
    m_fftPool.reserve(100000);
}

void MeasurementDataProcessor::onRawPacketArrived(int /*externalType*/, const QByteArray& data)
{
    // 1. 基础检查
    if (!m_status.is(ThreadWorkState::Working)) return;
    if (data.size() < 8) return;
    if ((uchar)data.at(0) != 0xBB) return;

    uchar typeFlag = (uchar)data.at(1);
    quint8 currentSeq = (quint8)data.at(3); // 读取 UDP 包计数

    // =================================================================
    // 【核心修复 1】：全局维护序号
    // 不管是 ADC(D0) 还是 FFT(D1)，只要包到了，就说明链路没断
    // =================================================================
    if (!m_firstSeqReceived) {
        m_firstSeqReceived = true;
        m_lastSeq = currentSeq;
    } else {
        // 计算差值
        int diff = (currentSeq - m_lastSeq + 256) % 256;

        // 正常情况 diff 应该等于 1 (连续)
        // 如果 diff > 1，说明真有包在网络里丢了
        if (diff > 1) {
            int lost = diff - 1;
            // 只打印警告，不再瞎补时间，防止出现"平顶怪线"
            // qWarning() << "[Network] 真实丢包侦测! 丢失数量:" << lost;
        }
        m_lastSeq = currentSeq; // 更新上一帧序号
    }

    const int HEADER_LEN = 4;
    const uchar* purePayloadPtr = reinterpret_cast<const uchar*>(data.constData() + HEADER_LEN);

    QMutexLocker lock(&m_dataMutex);

    // ================== ADC 处理 (D0) ==================
    if (typeFlag == 0xD0) {
        int count = 500;
        if (data.size() < HEADER_LEN + count * 2) return;

        if (m_adcPool.size() + count > MAX_POOL_SIZE) m_adcPool.clear();
        int oldSize = m_adcPool.size();
        m_adcPool.resize(oldSize + count);
        for (int i = 0; i < count; ++i) {
            m_adcPool[oldSize + i] = qFromBigEndian<quint16>(purePayloadPtr + i * 2);
        }
    }
    // ================== FFT 处理 (D1) ==================
    else if (typeFlag == 0xD1) {
        // 【核心修复 2】：移除时间补偿逻辑
        // 既然无法确定丢的是什么包，就严格按收到的数据画图，保证波形连续美观
        const double FIXED_STEP = 0.5;
        int count = 41;

        if (data.size() < HEADER_LEN + count * 4) return;

        QVector<QPointF> batchPoints;
        batchPoints.reserve(count);

        for (int i = 0; i < count; ++i) {
            quint32 val = qFromBigEndian<quint32>(purePayloadPtr + i * 4);

            // 无条件存储
            m_fftPool.append(val);

            // 构造点
            batchPoints.append(QPointF(m_totalElapsedTime, static_cast<double>(val)));
            m_totalElapsedTime += FIXED_STEP;
        }

        emit waveformUpdated(batchPoints);
    }
}

Q_INVOKABLE void MeasurementDataProcessor::getAndClearPools(QVector<quint16>& adcOut, QVector<quint32>& fftOut)
{
    QMutexLocker lock(&m_dataMutex);
    qDebug() << "[Processor-Export] 收到提取请求, 当前池子容量 - ADC:" << m_adcPool.size()
             << " FFT:" << m_fftPool.size();

    if (!m_fftPool.isEmpty()) {
        qDebug() << "[Processor-Export] FFT 样例数据(应为25M级整数):" << m_fftPool.last();
    }

    adcOut.swap(m_adcPool);
    fftOut.swap(m_fftPool);

    m_adcPool.reserve(5 * 1024 * 1024);
    m_fftPool.reserve(500 * 1024);
    qDebug() << "[Processor-Export] 数据交换(swap)完成";
}

void MeasurementDataProcessor::requestStart()
{
    // A. 先检查状态
    if (m_status.is(ThreadWorkState::Working)) return;

    // B. 执行清空和预分配
    resetPools();

    // C. 启动时钟
    m_sessionClock.start();

    // D. 通知 Worker 开始采集
    emit startAcquisitionRequested();

    m_status.set(ThreadWorkState::Working);
}

void MeasurementDataProcessor::resetPools()
{
    QMutexLocker lock(&m_dataMutex);

    // 1. 彻底释放旧内存并清空
    m_adcPool.clear();
    m_fftPool.clear();
    m_cycleBuffer.clear();
    m_timeBuffer.clear();
    m_totalElapsedTime = 0.0;

    // 2. 预分配空间（根据您的采样率预估，比如预留 20 秒的数据量）
    // ADC: 5000包/秒 * 500点/包 = 2.5M点/秒
    m_adcPool.reserve(5 * 1024 * 1024);

    // FFT: 只有164字节有效，内存压力小，但也预留一些
    m_fftPool.reserve(500 * 1024);

    // 3. 重置解析相关的状态位
    m_firstPacket = true;
    m_lostCount = 0;
    m_firstSeqReceived = false; // 重置序号状态
    m_lastSeq = 0;
}

void MeasurementDataProcessor::requestStop()
{
    // 1. 检查状态，防止重复停止
    if (!m_status.is(ThreadWorkState::Working)) return;

    // 2. 修改状态位，停止 onRawPacketArrived 的继续写入
    m_status.set(ThreadWorkState::Idle);

    // 3. (可选) 如果需要通知 Worker 线程物理停止 UDP 接收，可以发信号
    // emit stopAcquisitionRequested();

//    qInfo() << "[Processor] 停止量测，当前 ADC 池点数:" << m_adcPool.size();

    // 4. 发出完成信号，可以带上当前的数据结果统计
//    emit measureFinished(result);
}

// 在处理函数中实现
void MeasurementDataProcessor::processSignal(double timestamp, double rawValue) {
    // 1. 构造单个数据点：X为全局累计毫秒，Y为原始Hz数值
    // 注意：这里不再除以 1000000.0，保持后端原始 Hz 发送
    QPointF currentPoint(timestamp, rawValue);

    // 2. 构造一个临时的 QVector。
    // 因为 UI 端的 setSimpleData 接口接收的是数组，这样可以保持兼容性
    QVector<QPointF> pointData;
    pointData.append(currentPoint);

    // 3. 实时发射。UI 端的扫描棒会随着每个点位数据的到来即时向右推进
    // 这将完美配合 120ms 的扫描窗口实现示波器效果
    emit waveformUpdated(pointData);
}
