#include "deviceacquisitionworker.h"
#include <QThread>

DeviceAcquisitionWorker::DeviceAcquisitionWorker(QObject* parent)
    : QObject(parent)
{
    m_workingBatch.reserve(8192);
    qInfo() << "[DeviceWorker] ctor thread=" << QThread::currentThreadId();
}

DeviceAcquisitionWorker::~DeviceAcquisitionWorker()
{
    if (drainTimer_) {
        drainTimer_->stop();
        drainTimer_->deleteLater();
        drainTimer_ = nullptr;
    }
    if (udpManager_) {
        udpManager_->deleteLater();
        udpManager_ = nullptr;
    }
}

void DeviceAcquisitionWorker::initUdpManager()
{
    if (udpManager_) return;

    udpManager_ = new UdpManager(this);

    if (!udpManager_->startListening()) {
        emit acquisitionError("UDP bind/listen failed");
        state_ = State::Error;
        return;
    }

    // 批处理：1ms 定时 drain（高密度更稳）
    drainTimer_ = new QTimer(this);
    drainTimer_->setTimerType(Qt::PreciseTimer);
    connect(drainTimer_, &QTimer::timeout,
            this, &DeviceAcquisitionWorker::drainPackets,
            Qt::DirectConnection);

    drainTimer_->start(1); // 可试 0 或 1

    qInfo() << "[DeviceWorker] UDP ready thread=" << QThread::currentThreadId();
}

void DeviceAcquisitionWorker::requestStart()
{

    if (state_ != State::Idle) return;

    if (!udpManager_) {

        // 如果这里是野指针但非空，下面就会崩
        emit acquisitionError("UDP not initialized");
        state_ = State::Error;
        return;
    }

    QByteArray startCmd = buildStartFrame();

    bool ret = udpManager_->sendData(startCmd);

    if (ret) {

        udpManager_->setSessionActive(true);

        state_ = State::Working;

        // 保留一个 Qt 日志以便查看信号槽关联
        qInfo() << "[DeviceWorker] acquisition started";
    }
}

void DeviceAcquisitionWorker::requestStop()
{
    if (state_ != State::Working) return;

    udpManager_->sendData(buildStopFrame());
    udpManager_->setSessionActive(false);

    state_ = State::Idle;
    emit stopAcquisitionRequested();
    qInfo() << "[DeviceWorker] acquisition stopped";
}

bool DeviceAcquisitionWorker::isValidFrame(const QByteArray& p) const
{
    if (p.size() != kFullPacketSize) return false;

    uint8_t h0 = (uint8_t)p[0];
    uint8_t h1 = (uint8_t)p[1];

    // 同时兼容 ADC(bbd0) 和 FFT(bbd1)
    bool isAdc = (h0 == kAdcHeader0 && h1 == kAdcHeader1);
    bool isFft = (h0 == kFftHeader0 && h1 == kFftHeader1);

    return isAdc || isFft;
}

// 对应协议：标志(2字节) + ADC计数(1字节) + UDP计数(1字节)
// 所以 UDP 计数位的偏移量是 3
uint8_t DeviceAcquisitionWorker::getCount(const QByteArray& p) const
{
    // 确保安全访问，防止空包或短包导致越界崩溃
    if (p.size() < 4) return 0;

    // 返回第四个字节作为丢包校验的主序号
    return (uint8_t)p[3];
}

int8_t DeviceAcquisitionWorker::diff8(uint8_t curr, uint8_t expected)
{
    return (int8_t)(curr - expected); // 256 回绕
}

void DeviceAcquisitionWorker::drainPackets()
{
    Q_ASSERT(QThread::currentThread() == this->thread());
    if (state_ != State::Working || !udpManager_) return;

    m_workingBatch.clear();

    // 1. 批量消纳数据包
    while (true) {
        int got = udpManager_->popBatch(m_workingBatch, 4096);
        if (got <= 0) break;

        for (int i = 0; i < m_workingBatch.size(); ++i) {
            handleFrame(m_workingBatch[i]); // 在这里面累计各计数器
        }
        m_workingBatch.clear();
    }

    // 2. 严格控制频率：每秒仅打印一次汇总日志
    if (statTimer_.elapsed() >= 1000) {
        // 获取实时队列余量，用于判断 dropQueue
        uint32_t qSize = udpManager_->queueSize();

        // 格式化输出您要求的核心指标
        qInfo() << QString("[Health Report] lossEvents=%1 lostTotal=%2 outOfOrder=%3 badHeader=%4 badLen=%5 dropQueue=%6")
                   .arg(lossEvents_)
                   .arg(lostTotal_)
                   .arg(outOfOrder_) // 若 handleFrame 中检测到乱序可填入
                   .arg(badHeader_)
                   .arg(badLen_)
                   .arg(qSize); // 这里用队列大小来直观反映是否有溢出风险

        // 3. 统计清零，开始新一秒的监测
        lossEvents_ = lostTotal_ = badHeader_ = badLen_ = outOfOrder_ = 0;
        statTimer_.restart();
    }
}

void DeviceAcquisitionWorker::handleFrame(const QByteArray& packet)
{
    // 1. 物理长度监测 (保持原有逻辑)
    const int actualSize = packet.size();
    if (actualSize != kFullPacketSize) {
        badLen_++;
        return;
    }

    const uchar* rawPtr = reinterpret_cast<const uchar*>(packet.constData());
    int headerPos = -1;

    // 2. 动态找头并识别类型
    bool isAdc = false;
    bool isFft = false;

    for (int i = 0; i < 10; ++i) {
        if (rawPtr[i] == 0xBB) {
            if (rawPtr[i+1] == 0xD0) {
                headerPos = i;
                isAdc = true;
                break;
            } else if (rawPtr[i+1] == 0xD1) {
                headerPos = i;
                isFft = true;
                break;
            }
        }
    }

    if (headerPos == -1) {
        badHeader_++;
        return;
    }

    // 3. 序号校验 (保持原有逻辑，确保监测丢包)
    uint8_t currUdpSeq = rawPtr[headerPos + 3];
    if (!seqInited_) {
        expectedSeq_ = (uint8_t)(currUdpSeq + 1);
        seqInited_ = true;
    } else {
        if (currUdpSeq != expectedSeq_) {
            int gap = (uint8_t)(currUdpSeq - expectedSeq_);
            lostTotal_ += gap;
            lossEvents_++;
        }
        expectedSeq_ = (uint8_t)(currUdpSeq + 1);
    }

    int type = isAdc ? 0 : 1;
    emit rawPacketArrived(type, packet);
}

QByteArray DeviceAcquisitionWorker::buildStartFrame() const
{
    return QByteArray::fromHex("AA001700010D0A");
}

QByteArray DeviceAcquisitionWorker::buildStopFrame() const
{
    return QByteArray::fromHex("AA001700000D0A");
}
