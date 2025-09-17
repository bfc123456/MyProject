
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

/**
 * @struct WaveParams
 * @brief 存储一次完整波形计算得到的关键生理参数
 *
 * WaveParams 封装了血压波形分析后得到的主要结果：
 * - 收缩压最大值（SPAP）
 * - 舒张压最小值（DPAP）
 * - 平均压（MPAP）
 * - 心率
 *
 * 这些参数通常在测量结束后，通过对完整波形进行计算得出。
 */
struct WaveParams {
    float maxSPAP = 0.0f;   // 收缩压最大值
    float minDPAP = 0.0f;   // 舒张压最小值
    float avgMPAP = 0.0f;   // 平均压
    float heartRate = 0.0f; // 心率
};

/**
 * @class MeasurementDataProcessor
 * @brief 测量数据处理器
 *
 * MeasurementDataProcessor 负责接收实时传感器数据，进行波形解析、
 * 峰值检测、心率计算和最终参数提取。该类通常运行在独立线程中，
 * 通过 Qt 信号槽机制将解析结果传递给 UI 或数据存储模块。
 *
 * 核心功能：
 * - 通过 parseData() 槽函数接收实时采样点
 * - 检测峰值并估算心率
 * - 在测量结束时计算最终的收缩压、舒张压、平均压等参数
 * - 周期性发射批量波形数据用于 UI 绘制
 *
 * 信号：
 * - dataParsed()：发射单次解析结果
 * - measureFinished()：发射一次完整测量结果
 * - processingStarted()：处理线程启动时发射
 *
 * 内部机制：
 * - 使用 QMutex 保证多线程环境下数据安全
 * - 通过 QVector<QPointF> 存储完整波形 (x: 时间, y: 压力)
 * - 使用 QElapsedTimer 精确记录时间戳
 */

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
