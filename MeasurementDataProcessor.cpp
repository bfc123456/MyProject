#include "measurementdataprocessor.h"
#include <algorithm>
#include <QDebug>
#include <QtEndian>

MeasurementDataProcessor::MeasurementDataProcessor(QObject* parent)
    : QObject(parent)
{
    qDebug() << "[Processor] ctor, thread:" << QThread::currentThreadId();
}

MeasurementDataProcessor::~MeasurementDataProcessor()
{
    qDebug() << "[Processor] dtor";
}

void MeasurementDataProcessor::requestStart()
{
    qInfo() << "[Processor] === requestStart ===";
    qInfo() << "[Processor] 当前状态：" << static_cast<int>(m_state.get());
    qInfo() << "[Processor] 线程ID：" << QThread::currentThreadId();

    if (!m_state.is(ThreadWorkState::Idle)) {
        qWarning() << "[Processor] 状态不是Idle，无法启动";
        return;
    }

    m_state.set(ThreadWorkState::Starting);  // 先设置为Starting
    qInfo() << "[Processor] 状态设置为Starting";

    resetSession();
    m_sessionClock.start();
    m_uiClock.invalidate();

    // TODO: 这里应该通知 DeviceAcquisitionWorker 开始采集
    // 例如：emit startAcquisitionRequested();

    m_state.set(ThreadWorkState::Working);  // 最后设置为Working
    qInfo() << "[Processor] 状态设置为Working";
    qInfo() << "[Processor] start processing";
}

void MeasurementDataProcessor::requestStop()
{
    // Busy=Working/Stopping 都算忙，你之前用 ThreadWorkStateIsBusy 也行
    if (!ThreadWorkStateIsBusy(m_state.get()))
        return;

    m_state.set(ThreadWorkState::Stopping);

    // 结束前最后flush一次UI缓冲（可选但推荐）
    tryFlushUiFrame();

    WaveParams params = calculateFinalParams();

    MeasurementData result;
    result.sensorSystolic  = QString::number(params.maxSPAP, 'f', 2);
    result.sensorDiastolic = QString::number(params.minDPAP, 'f', 2);
    result.sensorAvg       = QString::number(params.avgMPAP, 'f', 2);
    result.heartRate       = QString::number(params.heartRate, 'f', 1);

    // 这里放用于“最终展示/保存”的波形（如果你只抽样显示，建议用别的字段存）
    result.points = m_waveformForParams;

    m_state.set(ThreadWorkState::Idle);
    emit measureFinished(result);

    qInfo() << "[Processor] processing stopped";
}

void MeasurementDataProcessor::resetSession()
{
    QMutexLocker lock(&m_mutex);
    m_rawQueue.clear();
    m_fftQueue.clear();

    m_uiBuffer.clear();
    m_waveformForParams.clear();
}

void MeasurementDataProcessor::storeRawPacket(const QByteArray& data)
{
    QMutexLocker lock(&m_mutex);
    m_rawQueue.enqueue(data);  // QByteArray隐式共享，入队很轻
}

void MeasurementDataProcessor::storeFftPacket(const QByteArray& data)
{
    QMutexLocker lock(&m_mutex);
    m_fftQueue.enqueue(data);
}

void MeasurementDataProcessor::onRawPacketArrived(const QByteArray& data)
{
    if (!m_state.is(ThreadWorkState::Working))
        return;

    // 1) 全量存储（导出用）
    storeRawPacket(data);
}

void MeasurementDataProcessor::onFftPacketArrived(const QByteArray& data)
{
    if (!m_state.is(ThreadWorkState::Working))
        return;

    // 1) 全量存储（导出用）
    storeFftPacket(data);

    // 2) 从FFT抽样生成用于UI显示的点（新对象，不影响存储）
    const QVector<QPointF> pts = sampleFftForUi(data);
    if (!pts.isEmpty())
        appendUiPointsAndMaybeFlush(pts);

    // 3) 如果你最终参数也要基于“显示波形”，你可以同时维护一份用于参数的波形
    // 注意：这份是抽样波形，不一定适合算血压心率（真实项目应从RAW解析）
    m_waveformForParams += pts;
}

QVector<QPointF> MeasurementDataProcessor::sampleFftForUi(const QByteArray& fftPkt) const
{
    QVector<QPointF> out;
    if (fftPkt.size() < 4) return out;

    // 假设FFT是16-bit序列
    const int n = fftPkt.size() / 2;

    // 端序你一定要确认：下位机发的是小端还是大端
    // 这里先按小端
    auto readU16 = [&](int idx) -> quint16 {
        const uchar* p = reinterpret_cast<const uchar*>(fftPkt.constData() + idx * 2);
        return qFromLittleEndian<quint16>(p);
    };

    const double t = m_sessionClock.isValid() ? (m_sessionClock.elapsed() / 1000.0) : 0.0;

    // 抽样策略：固定选几个bin（你可换成“每隔step取一个”）
    static const int bins[] = { 5, 10, 20, 30, 40, 60, 80, 100 };

    out.reserve(int(sizeof(bins)/sizeof(bins[0])));
    for (int b : bins) {
        if (b >= 0 && b < n) {
            const double y = double(readU16(b));
            out.append(QPointF(t, y));
        }
    }
    return out;
}

void MeasurementDataProcessor::appendUiPointsAndMaybeFlush(const QVector<QPointF>& pts)
{
    // 这几个slot都在Processor线程执行，所以m_uiBuffer不需要锁
    m_uiBuffer += pts;
    tryFlushUiFrame();
}

void MeasurementDataProcessor::tryFlushUiFrame()
{
    if (m_uiBuffer.isEmpty())
        return;

    // 第一次触发时启动节流计时
    if (!m_uiClock.isValid())
        m_uiClock.start();

    const bool timeOk  = (m_uiClock.elapsed() >= UI_PUSH_MS);
    const bool countOk = (m_uiBuffer.size() >= UI_PUSH_N);

    if (!(timeOk || countOk))
        return;

    QVector<QPointF> frame;
    frame.swap(m_uiBuffer);     // swap：几乎零拷贝，不影响存储
    m_uiClock.restart();

    emit waveformUpdated(frame); // UI线程Queued接收
}

void MeasurementDataProcessor::takeAllQueues(QQueue<QByteArray>& outRaw, QQueue<QByteArray>& outFft)
{
    QMutexLocker lock(&m_mutex);
    outRaw.swap(m_rawQueue);
    outFft.swap(m_fftQueue);
}

// ===================== 最终参数计算（沿用你原来的思路） =====================

WaveParams MeasurementDataProcessor::calculateFinalParams()
{
    WaveParams r;
    if (m_waveformForParams.isEmpty())
        return r;

    QVector<float> v;
    v.reserve(m_waveformForParams.size());
    for (const auto& p : m_waveformForParams)
        v.push_back(float(p.y()));

    r.maxSPAP = *std::max_element(v.begin(), v.end());
    r.minDPAP = *std::min_element(v.begin(), v.end());
    r.avgMPAP = r.minDPAP + (r.maxSPAP - r.minDPAP) / 3.f;

    QList<int> peaks = detectPeaks(m_waveformForParams);
    if (peaks.size() >= 2) {
        double total = 0.0;
        for (int i = 1; i < peaks.size(); ++i)
            total += (m_waveformForParams[peaks[i]].x() - m_waveformForParams[peaks[i-1]].x());

        const double avgCycle = total / double(peaks.size() - 1);
        if (avgCycle > 1e-6)
            r.heartRate = float(60.0 / avgCycle);
    }
    return r;
}

QList<int> MeasurementDataProcessor::detectPeaks(const QVector<QPointF>& w)
{
    QList<int> peaks;
    for (int i = 2; i < w.size() - 2; ++i) {
        if (w[i].y() > w[i-1].y() && w[i].y() > w[i+1].y())
            peaks << i;
    }
    return peaks;
}
