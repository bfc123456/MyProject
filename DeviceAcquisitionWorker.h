#ifndef DEVICEACQUISITIONWORKER_H
#define DEVICEACQUISITIONWORKER_H

#include <QObject>
#include <QTimer>
#include <QElapsedTimer>
#include <QVector>
#include <cstdint>
#include <QPointer>

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
    // 定义转发给 Processor 的原始包信号
    void rawPacketArrived(int type, const QByteArray& packet);
    void acquisitionError(const QString& err);
    void stopAcquisitionRequested();

    // 如果你需要把解析后的点发给后续处理线程，可以启用这个信号（注意：别每包发给 UI）
    // void rawSamplesReady(QVector<int16_t> samples);

private slots:
    void drainPackets();

private:
    QByteArray buildStartFrame() const;
    QByteArray buildStopFrame() const;

    // --- 协议常量重定义 ---
    static const int kFullPacketSize = 1004;  // 总长 1004 字节
    static const int kHeaderSize     = 2;     // 标志位长度
    static const int kPayloadOffset  = 4;     // 标志(2) + ADC计(1) + UDP计(1)

    // 标志位定义
    static const uint8_t kAdcHeader0 = 0xBB;
    static const uint8_t kAdcHeader1 = 0xD0;
    static const uint8_t kFftHeader0 = 0xBB;
    static const uint8_t kFftHeader1 = 0xD1;

    inline bool isValidFrame(const QByteArray& p) const;
    inline uint8_t getCount(const QByteArray& p) const;
    static inline int8_t diff8(uint8_t curr, uint8_t expected);

    void handleFrame(const QByteArray& packet);

private:
    QPointer<UdpManager> udpManager_;
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

    // 【新增】类成员缓冲区，避免重复分配内存
    QVector<QByteArray> m_workingBatch;
};

#endif // DEVICEACQUISITIONWORKER_H
