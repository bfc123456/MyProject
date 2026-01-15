#ifndef DEVICEACQUISITIONWORKER_H
#define DEVICEACQUISITIONWORKER_H

#include <QObject>
#include <QTimer>
#include <QElapsedTimer>
#include <QVector>
#include <cstdint>

#include "udpmanager.h"

class DeviceAcquisitionWorker : public QObject
{
    Q_OBJECT
public:
    explicit DeviceAcquisitionWorker(QObject* parent = nullptr);
    ~DeviceAcquisitionWorker() override;

    void initUdpManager();

public slots:
    void requestStart();
    void requestStop();

signals:
    void acquisitionError(const QString& err);
    void acquisitionStopped();

    // 如果你需要把解析后的点发给后续处理线程，可以启用这个信号（注意：别每包发给 UI）
    // void rawSamplesReady(QVector<int16_t> samples);

private slots:
    void drainPackets();

private:
    QByteArray buildStartFrame() const;
    QByteArray buildStopFrame() const;

    // 协议常量：88 66 86 + count(1B)
    static constexpr uint8_t H0 = 0x88;
    static constexpr uint8_t H1 = 0x66;
    static constexpr uint8_t H2 = 0x86;
    static constexpr int kCountOff = 3;
    static constexpr int kPayloadOff = 4;

    // 选择你的 payload 格式：2=采集原始点；4=FFT float/int32
    static constexpr int kBytesPerPoint = 2;

    inline bool isValidFrame(const QByteArray& p) const;
    inline uint8_t getCount(const QByteArray& p) const;
    static inline int8_t diff8(uint8_t curr, uint8_t expected);

    void handleFrame(const QByteArray& packet);

private:
    UdpManager* udpManager_ = nullptr;
    QTimer* drainTimer_ = nullptr;

    enum class State { Idle, Working, Error };
    State state_ = State::Idle;

    // 计数回绕统计
    bool seqInited_ = false;
    uint8_t expectedSeq_ = 0;

    uint64_t lossEvents_ = 0;
    uint64_t lostTotal_  = 0;
    uint64_t outOfOrder_ = 0;
    uint64_t badHeader_  = 0;
    uint64_t badLen_     = 0;

    QElapsedTimer statTimer_;
};

#endif // DEVICEACQUISITIONWORKER_H
