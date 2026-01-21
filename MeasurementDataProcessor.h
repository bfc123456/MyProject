#ifndef MEASUREMENTDataProcessor_H
#define MEASUREMENTDataProcessor_H

#include <QObject>
#include <QByteArray>
#include <QVector>
#include <QPointF>
#include <QMutex>
#include <QElapsedTimer>
#include <QQueue>
#include "measurementdata.h"

enum class ThreadWorkState { Idle, Starting, Working, Stopping, Error };

struct CycleFeatures {
    double timestamp;
    double maxValue;
    double minValue;
    QVector<QPointF> scatteredPoints; // 存储周期内的其他关键点
};

//  告诉 Qt 这个结构体要跨线程传输
Q_DECLARE_METATYPE(CycleFeatures)

class MeasurementDataProcessor : public QObject
{
    Q_OBJECT
public:
    explicit MeasurementDataProcessor(QObject* parent = nullptr);

    // 供 UI 定时拉取的接口（替代 takeAllQueues，效率更高）
    Q_INVOKABLE void getAndClearPools(QVector<quint16>& adcOut, QVector<quint32>& fftOut);

public slots:
    void requestStart();
    void requestStop();
    void onRawPacketArrived(int type, const QByteArray& packet);

signals:
    void waveformUpdated(const QVector<QPointF> &data);
    void measureFinished(const MeasurementData& result);
    void startAcquisitionRequested();

private:
    void resetPools();
    void processSignal(double timestamp, double rawValue);

    // 状态管理
    struct {
        ThreadWorkState state { ThreadWorkState::Idle };
        bool is(ThreadWorkState s) const { return state == s; }
        void set(ThreadWorkState s) { state = s; }
    } m_status;

    QMutex m_dataMutex;
    QByteArray m_parseBuffer;

    // --- 极致性能区：预分配的数值池 ---
    QVector<quint16> m_adcPool; // 存储解析后的 ADC 数值
    QVector<quint32> m_fftPool; // 存储解析后的 FFT 数值
    const int MAX_POOL_SIZE = 5 * 1024 * 1024; // 约 500 万点，防止内存爆炸

    // --- UI 节流相关 ---
    QVector<QPointF> m_uiBuffer;
    QElapsedTimer m_sessionClock;
    QElapsedTimer m_uiClock;

    // 丢包统计（仅记录，不频繁打印）
    quint8 m_lastUdpSeq = 0;
    bool m_firstPacket = true;
    quint64 m_lostCount = 0;

    static const int FULL_PACKET_SIZE = 1004;
    static const int UI_REFRESH_MS = 33;
    static const int UI_MAX_POINTS = 1000;
    double m_totalElapsedTime = 0.0;

    // ---用于周期最值提取的私有变量 ---
    QVector<double> m_cycleBuffer;   // 周期原始值缓冲区
    QVector<double> m_timeBuffer;    // 对应时间戳缓冲区
    int m_pointsPerCycle = 800;      // 一个心律周期的预估点数（根据实际采样率调整）
    quint8 m_lastSeq = 0;      // 记录上一个包的序号
    bool m_firstSeqReceived = false; // 是否是收到的第一个包

};

#endif
