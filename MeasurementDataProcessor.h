
#ifndef MEASUREMENTDataProcessor_H
#define MEASUREMENTDataProcessor_H

#include <QThread>
#include <QByteArray>
#include <QDebug>
#include <QElapsedTimer>
#include <QQueue>
#include "measurementdata.h"
#include <QRandomGenerator>
#include <QElapsedTimer>
#include <QMutex>
#include <QMutexLocker>
#include "measurementdata.h"
#include "measurementconfig.h"

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
enum class ThreadWorkState
{
    Idle,
    Starting,
    Working,
    Stopping,
    Error
};

inline bool ThreadWorkStateIsBusy(ThreadWorkState s)
{
    return (s == ThreadWorkState::Starting ||
            s == ThreadWorkState::Working  ||
            s == ThreadWorkState::Stopping);
}

class AtomicState
{
public:
    ThreadWorkState get() const { return m_state; }
    void set(ThreadWorkState s) { m_state = s; }
    bool is(ThreadWorkState s) const { return m_state == s; }
private:
    ThreadWorkState m_state { ThreadWorkState::Idle };
};

// ===================== 数据结构 =====================
struct WaveParams
{
    float maxSPAP   = 0.f;
    float minDPAP   = 0.f;
    float avgMPAP   = 0.f;
    float heartRate = 0.f;
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
    // 由 UI / Controller 调用
    void requestStart();
    void requestStop();

    // 由接收线程调用（QueuedConnection）
    void onRawPacketArrived(const QByteArray& data);
    void onFftPacketArrived(const QByteArray& data);

public:
    // 导出时用：一次性“取走”所有包（swap，不复制，不影响实时）
    void takeAllQueues(QQueue<QByteArray>& outRaw, QQueue<QByteArray>& outFft);

signals:
    // 注意：这里不再发“全量波形”，而是发“一个UI帧”（节流后的批量点）
    void waveformUpdated(const QVector<QPointF>& frame);

    // 测量结束（你原来的）
    void measureFinished(const MeasurementData& result);

    void processingError(const QString& msg);

private:
    void resetSession();

    // ===== 存储（全量） =====
    void storeRawPacket(const QByteArray& data);
    void storeFftPacket(const QByteArray& data);

    // ===== UI波形（抽样 + 节流） =====
    QVector<QPointF> sampleFftForUi(const QByteArray& fftPkt) const;  // FFT抽样策略
    void appendUiPointsAndMaybeFlush(const QVector<QPointF>& pts);
    void tryFlushUiFrame(); // 条件触发：20ms 或 200点

    // ===== 最终参数计算（可沿用你现有的算法） =====
    WaveParams calculateFinalParams();
    QList<int> detectPeaks(const QVector<QPointF>& waveform);

private:
    AtomicState m_state;
    QMutex m_mutex;

    // 全量缓存（导出用）
    QQueue<QByteArray> m_rawQueue;
    QQueue<QByteArray> m_fftQueue;

    // 如果你最终参数要用波形，可保留完整波形（注意：如果只抽样用于显示，参数可能不准）
    QVector<QPointF> m_waveformForParams;

    // UI节流缓冲（只用于显示）
    QVector<QPointF> m_uiBuffer;
    QElapsedTimer m_sessionClock; // 测量起点
    QElapsedTimer m_uiClock;      // 节流计时

    static constexpr int UI_PUSH_MS = 80;   // 20ms推一次
    static constexpr int UI_PUSH_N  = 120;  // 或累计200点推一次
};

#endif // MEASUREMENTDataProcessor_H
