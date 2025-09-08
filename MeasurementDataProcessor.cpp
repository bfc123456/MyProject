#include "MeasurementDataProcessor.h"
#include "MeasurementDialog.h"  // 引入界面宏 MD_PROGRESS_TOTAL_DURATION
#include "MeasurementData.h"
#include <algorithm>
#include <QMutexLocker>
#include <QDebug>

//构造函数：初始化状态
MeasurementDataProcessor::MeasurementDataProcessor(QObject* parent) : QObject(parent)
{
    resetMeasurementState();
    qDebug() << "[MeasurementDataProcessor] 构造函数（主线程ID：" << QThread::currentThreadId() << "）";
}

//析构函数：清理资源
MeasurementDataProcessor::~MeasurementDataProcessor()
{
    qDebug() << "[MeasurementDataProcessor] 析构，清理状态";
}

// 重置测量状态（所有变量归零）
void MeasurementDataProcessor::resetMeasurementState()
{
    m_waveformData.clear();
    qDebug() << "[resetMeasurementState] 清空后 m_waveformData 大小：" << m_waveformData.size();
    m_tempMaxSPAP = m_tempMinDPAP = 0;
    m_tempHeartRate = 0.0;
    m_lastPeakTime = 0;
    m_peakThreshold = 50; // 阈值初始化，避免随机值

    m_t0_ns = 0;
    m_lastPkt_ns = 0;
}

//启动/停止测量（线程安全）
void MeasurementDataProcessor::setMeasuring(bool shouldMeasure)
{
    QMutexLocker locker(&m_mutex);
      if (m_isMeasuring == shouldMeasure) return; // 状态无变化，直接返回

      m_isMeasuring = shouldMeasure;
      if (shouldMeasure) {
          // 开始测量：重置状态 + 记录启动时间
          resetMeasurementState();
          m_elapsed.start();   // 基准：点击按钮时刻
//          m_measureStartMs = measureStartMs;
          qDebug() << "[MeasurementDataProcessor] 测量启动";
          emit processingStarted();
      } else {
          // ——拷贝数据后解锁——
          QVector<QPointF> waveformCopy = m_waveformData;

          // 停止测量：先计算准确参数，再通知结束
          WaveParams finalParams = calculateFinalParams();
          locker.unlock();

          qDebug() << "[测量结束] 准确参数：SPAP=" << finalParams.maxSPAP
                   << " DPAP=" << finalParams.minDPAP
                   << " 心率=" << finalParams.heartRate;

          // 发送最终准确结果（可扩展信号通知UI更新）
          MeasurementData finalResult;
          finalResult.sensorSystolic = QString::number(finalParams.maxSPAP, 'f', 2);
          finalResult.sensorDiastolic = QString::number(finalParams.minDPAP, 'f', 2);
          finalResult.sensorAvg = QString::number(finalParams.avgMPAP, 'f', 2);
          finalResult.heartRate = QString::number(finalParams.heartRate, 'f', 1);
          finalResult.points = m_waveformData;
//          emit dataParsed(finalResult);
          emit measureFinished(finalResult);
  }
}

// 准确峰值检测（基于完整波形，测量结束后调用）
QList<int> MeasurementDataProcessor::detectPeaks(const QVector<QPointF>& waveform)
{
    QList<int> peaks;
    if (waveform.size() < 20) return peaks; // 至少20个点才检测

    // 动态阈值：波形前20%数据的平均值×1.5（适应不同振幅）
    double avgBase = 0.0;
    int baseCount = waveform.size() / 5;
    for (int i = 0; i < baseCount; ++i) avgBase += waveform[i].y();
    avgBase /= baseCount;
    float threshold = static_cast<float>(avgBase * 1.5);

    // 遍历找峰值：局部最大值 + 超过阈值 + 时间连续（排除跳变）
    for (int i = 3; i < waveform.size() - 3; ++i) {
        float curr = waveform[i].y();
        bool isLocalMax = (curr > waveform[i-1].y()) && (curr > waveform[i+1].y())
                        && (curr > waveform[i-2].y()) && (curr > waveform[i+2].y());
        bool isOverThreshold = curr > threshold;
        // 时间间隔检查（相邻点不超过100ms，排除异常数据）
        double timeDiff = waveform[i].x() - waveform[i-1].x();
        bool isTimeContinuous = timeDiff < 0.1;

        if (isLocalMax && isOverThreshold && isTimeContinuous) {
            peaks.append(i);
        }
    }
    return peaks;
}

// 基于完整波形计算准确参数（后台复杂计算）
WaveParams MeasurementDataProcessor::calculateFinalParams()
{
    WaveParams result;
    if (m_waveformData.isEmpty()) return result;  // 无数据，直接返回

    // 提取压力值，计算全局最大/最小
    QVector<float> pressures;
    for (const auto& pt : m_waveformData) pressures.append(pt.y());
    result.maxSPAP = *std::max_element(pressures.begin(), pressures.end());
    result.minDPAP = *std::min_element(pressures.begin(), pressures.end());
    result.avgMPAP = result.minDPAP + (result.maxSPAP - result.minDPAP) / 3.0f;

    // 检测峰值，计算心率
    QList<int> peaks = detectPeaks(m_waveformData);
    if (peaks.size() >= 2) {
        double totalCycle = 0.0;
        int validCycles = 0;
        for (int i = 0; i < peaks.size() - 1; ++i) {
            double startTime = m_waveformData[peaks[i]].x();
            double endTime = m_waveformData[peaks[i+1]].x();
            double cycle = endTime - startTime;
            if (cycle >= 0.5 && cycle <= 2.0) {  // 过滤异常周期
                totalCycle += cycle;
                validCycles++;
            }
        }
        if (validCycles > 0) {
            result.heartRate = static_cast<float>(60.0 / (totalCycle / validCycles));
        }
    }
    return result;
}

// 实时数据解析（移除冗余调试，强化线程安全）
void MeasurementDataProcessor::parseData(quint32 currentValue)
{
//    qint64 nowMs = m_elapsed.elapsed();    // 单调毫秒（QElapsedTimer）

//    QMutexLocker locker(&m_mutex);  // 锁保护共享状态
    if (!m_isMeasuring) return;     // 非测量状态，直接过滤

    // 计算相对时间（基于测量启动时间）
    double relativeTime = m_elapsed.elapsed() / 1000.0; // 毫秒→秒

    const QPointF pt(relativeTime, double(currentValue));

    // 1) 先把“当前点”存起来（批量 + 可选全量）
    {
        QMutexLocker lk(&m_mutex);                  // 若本类只在一个线程用，可去掉锁
        m_batch.append(pt);                         // ☆ 发UI用的批量缓冲
        m_waveformData.append(pt);                  // 可选：全量保存/导出用
    }

    // 2) 初始化批量计时器
    if (!m_batchClock.isValid()) m_batchClock.start();

    // 3) 到时间/数量阈值就打包 MeasurementData 发出去
    static constexpr int BATCH_MS = 10;   // 每30ms发一次
    static constexpr int BATCH_N  = 50;  // 或累计200点发一次
    bool timeUp = (m_batchClock.elapsed() >= BATCH_MS);
    bool sizeUp = (m_batch.size() >= BATCH_N);

    if (timeUp || sizeUp) {
        MeasurementData md;
        {
            QMutexLocker lk(&m_mutex);
            if (!m_batch.isEmpty()) {
                md.points = m_batch;      // 只填 points
                m_batch.clear();
                m_batchClock.restart();
            } else {
                return;                   // 没点就不发
            }
        }
        emit dataParsed(md);              // ☆ 一批发一次（UI 只重绘一次）
    }






//    m_waveformData.append(QPointF(relativeTime, float(currentValue)));

//    // 存储波形数据
//    m_waveformData.append(QPointF(relativeTime, static_cast<float>(currentValue)));
//    int dataSize = m_waveformData.size();

//    // 更新极值（收缩压/舒张压）
//    if (dataSize == 1) {
//        m_tempMaxSPAP = m_tempMinDPAP = currentValue;
//    } else {
//        if (currentValue > m_tempMaxSPAP) m_tempMaxSPAP = currentValue;
//        if (currentValue < m_tempMinDPAP) m_tempMinDPAP = currentValue;
//    }

//    // 心率计算（满足数据量条件时）
//    if (dataSize >= MIN_DATA_FOR_HR) {
//        bool isPeak = false;
//        if (currentValue >= m_peakThreshold && dataSize >= 2) {
//            float prevY = m_waveformData[dataSize - 2].y();
//            isPeak = (currentValue > prevY);
//        }
//        if (isPeak) {
//            if (m_lastPeakTime != 0) {
//                double cycle = (nowMs  - m_lastPeakTime) / 1000.0;
//                if (cycle >= 0.5 && cycle <= 2.0) {  // 合理周期范围
//                    m_tempHeartRate = 60.0 / cycle;
//                }
//            } else {
//                m_lastPeakTime = nowMs ;  // 记录首次峰值时间
//            }
//        }
//    }

    // 转发实时数据到UI
//    MeasurementData realtimeResult;
//    realtimeResult.points.append(QPointF(relativeTime, static_cast<float>(currentValue)));
//    realtimeResult.sensorSystolic = QString::number(m_tempMaxSPAP, 'f', 2);
//    realtimeResult.sensorDiastolic = QString::number(m_tempMinDPAP, 'f', 2);
//    realtimeResult.sensorAvg = QString::number(m_tempMinDPAP + (m_tempMaxSPAP - m_tempMinDPAP)/3.0, 'f', 2);
//    realtimeResult.heartRate = QString::number(m_tempHeartRate, 'f', 1);

//    locker.unlock();
//    emit dataParsed(realtimeResult);


    // 超时判断
    if (m_elapsed.elapsed() >= MEASUREMENT_TOTAL_DURATION_MS) {
//        locker.unlock();
        QMetaObject::invokeMethod(this, "setMeasuring",
                                  Qt::QueuedConnection,
                                  Q_ARG(bool, false));
    }
}
