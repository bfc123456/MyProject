
#ifndef MEASUREMENTDataProcessor_H
#define MEASUREMENTDataProcessor_H

#include <QThread>
#include <QByteArray>
#include <QDebug>
#include <QElapsedTimer>
#include "MeasurementData.h"
#include "DeviceAcquisitionWorker.h"
#include <QRandomGenerator>
#include <QElapsedTimer>
#include <QMutex>
#include <QMutexLocker>
#include "MeasurementData.h"
#include "MeasurementConfig.h"

// 波形参数计算结果
struct WaveParams {
    float maxSPAP = 0.0f;   // 收缩压最大值
    float minDPAP = 0.0f;   // 舒张压最小值
    float avgMPAP = 0.0f;   // 平均压
    float heartRate = 0.0f; // 心率
};

class MeasurementDataProcessor : public QObject
{
    Q_OBJECT

public:

    explicit MeasurementDataProcessor(QObject* parent = nullptr);
    ~MeasurementDataProcessor();

public slots:

    void setMeasuring(bool status);
    void parseData(quint32 currentValue);    // 接收发送线程传来的单个浮点数（核心槽函数）


signals:
    void dataParsed(const MeasurementData& result);
    void measureFinished(const MeasurementData& result);
    void processingStarted();   // 新增：线程开始时发射
//    void processingFinished();  // 新增：线程结束时发射

private:
    QMutex m_mutex; // 保护共享数据的线程安全锁
    qint64 m_measureStartMs = 0; // 仅保留一个时间变量，由外部传入
    QVector<QPointF> m_waveformData;  // 完整波形数据（时间x, 压力y）
    bool m_isMeasuring = false;       // 测量状态标记

    // 实时临时变量（轻量更新）
    quint32 m_tempMaxSPAP = 0;        // 实时临时收缩压
    quint32 m_tempMinDPAP = 0;        // 实时临时舒张压
    double m_tempHeartRate = 0.0;     // 实时临时心率
    qint64 m_lastPeakTime = 0;        // 上次峰值时间（毫秒）
    quint32 m_peakThreshold = 50;     // 峰值检测阈值（初始值50，可动态调整）
    const int MIN_DATA_FOR_HR = 10;   // 计算心率最小数据量（避免初始噪声）

    // 后台复杂计算（测量结束后调用）
    WaveParams calculateFinalParams(); // 基于完整波形计算准确参数
    QList<int> detectPeaks(const QVector<QPointF>& waveform); // 准确峰值检测
    void resetMeasurementState();      // 重置测量状态
    bool allowFirstPacket_{false};

    QElapsedTimer m_elapsed;   // 单调时钟
    qint64 m_t0_ns = 0;        // 首包时间基准
    qint64 m_lastPkt_ns       = 0;  // 最近一包（纳秒）

    //以下为测试使用
    QVector<QPointF> m_batch;
    QElapsedTimer    m_batchClock;
    static constexpr int BATCH_MS = 20;    // 每 20ms 发 UI 一次
    static constexpr int BATCH_N  = 200;   // 或者每 200 点发一次
};

#endif // MEASUREMENTDataProcessor_H
