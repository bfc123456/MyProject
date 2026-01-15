#include "deviceacquisitionworker.h"
#include <QThread>

DeviceAcquisitionWorker::DeviceAcquisitionWorker(QObject* parent)
    : QObject(parent)
{
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
    printf(">> [STEP 1] Entering requestStart\n"); fflush(stdout);
    if (state_ != State::Idle) return;
    if (!udpManager_) {
        printf(">> [STEP 1-Error] UDP Manager is null\n"); fflush(stdout);
        emit acquisitionError("UDP not initialized");
        state_ = State::Error;
        return;
    }
    printf(">> [STEP 2] Preparing to send data...\n"); fflush(stdout);

    QByteArray startCmd = buildStartFrame();
        printf(">> [STEP 2.5] Frame built. Size: %d\n", startCmd.size()); fflush(stdout);

        bool sendResult = udpManager_->sendData(startCmd);

        printf(">> [STEP 3] sendData returned: %d\n", sendResult); fflush(stdout);

        if (!sendResult) {
            printf(">> [STEP 3-Error] Send failed\n"); fflush(stdout);
            emit acquisitionError("Send START failed");
            state_ = State::Error;
            return;
        }

    udpManager_->setSessionActive(true);

    // reset
    seqInited_ = false;
    expectedSeq_ = 0;
    lossEvents_ = lostTotal_ = outOfOrder_ = badHeader_ = badLen_ = 0;
    statTimer_.invalidate();

    state_ = State::Working;
    printf(">> [STEP 4] SUCCESS! Acquisition started.\n"); fflush(stdout);
    qInfo() << "[DeviceWorker] acquisition started";
}

void DeviceAcquisitionWorker::requestStop()
{
    if (state_ != State::Working) return;

    udpManager_->sendData(buildStopFrame());
    udpManager_->setSessionActive(false);

    state_ = State::Idle;
    emit acquisitionStopped();
    qInfo() << "[DeviceWorker] acquisition stopped";
}

bool DeviceAcquisitionWorker::isValidFrame(const QByteArray& p) const
{
    return p.size() >= kPayloadOff &&
           (uint8_t)p[0] == H0 &&
           (uint8_t)p[1] == H1 &&
           (uint8_t)p[2] == H2;
}

uint8_t DeviceAcquisitionWorker::getCount(const QByteArray& p) const
{
    return (uint8_t)p[kCountOff];
}

int8_t DeviceAcquisitionWorker::diff8(uint8_t curr, uint8_t expected)
{
    return (int8_t)(curr - expected); // 256 回绕
}

void DeviceAcquisitionWorker::drainPackets()
{
    if (state_ != State::Working || !udpManager_) return;

    QVector<QByteArray> batch;
    batch.reserve(8192);

    // 建议：每次回调尽量把队列清掉（或加时间预算）
    while (true) {
        int got = udpManager_->popBatch(batch, 8192);
        if (got <= 0) break;

        for (int i = 0; i < batch.size(); ++i) {
            handleFrame(batch[i]);
        }
        batch.clear();
    }

    // 1s统计（你现在是每秒打印一次，而且每秒清零，这是对的）
    if (!statTimer_.isValid()) statTimer_.start();
    if (statTimer_.elapsed() >= 1000) {
        qInfo() << "[DeviceWorker][1s]"
                << "lossEvents=" << lossEvents_
                << "lostTotal="  << lostTotal_
                << "outOfOrder=" << outOfOrder_
                << "badHeader="  << badHeader_
                << "badLen="     << badLen_
                << "dropQueue="  << udpManager_->droppedByQueue()
                << "qSize="      << udpManager_->queueSize()
                << "thread="     << QThread::currentThreadId();

        lossEvents_ = lostTotal_ = outOfOrder_ = badHeader_ = badLen_ = 0;
        statTimer_.restart();
    }
}

void DeviceAcquisitionWorker::handleFrame(const QByteArray& packet)
{
    if (!isValidFrame(packet)) {
        badHeader_++;
        return;
    }

    // 计数（第4字节）
    const uint8_t curr = getCount(packet);

    if (!seqInited_) {
        expectedSeq_ = (uint8_t)(curr + 1);
        seqInited_ = true;
    } else {
        // curr/expected 都是 uint8_t
        uint8_t d = uint8_t(curr - expectedSeq_);   // mod 256

        if (d == 0) {
            // 正常：正好收到了 expectedSeq_
            expectedSeq_ = uint8_t(curr + 1);
        } else if (d < 128) {
            // curr 比 expected 超前 d -> 中间缺了 d 个包
            lossEvents_++;
            lostTotal_ += d;
            expectedSeq_ = uint8_t(curr + 1);       // 追上最新
        } else {
            // d>=128：更像乱序/重复/回绕附近的倒退
            outOfOrder_++;
            // 关键：乱序不要改 expectedSeq_，否则会“带偏”后续统计
        }
    }

//    // payload 解析（每包固定）
//    const int payloadBytes = packet.size() - kPayloadOff;
//    if (payloadBytes <= 0 || (payloadBytes % kBytesPerPoint) != 0) {
//        badLen_++;
//        return;
//    }

//    const char* payload = packet.constData() + kPayloadOff;
//        const int n = payloadBytes / kBytesPerPoint; // 数据点个数

//        if constexpr (kBytesPerPoint == 2) {
//            // --- 修正后的 int16 处理 ---
//            for (int i = 0; i < n; ++i) {
//                int16_t sample;
//                // 【安全拷贝】不管地址对不对齐，memcpy 都能正确读出 2 个字节
//                memcpy(&sample, payload + i * sizeof(int16_t), sizeof(int16_t));

//                // 如果涉及大小端转换（假设网络是大端，本机是小端）：
//                // sample = qFromBigEndian(sample);

//                // TODO: 将 sample 入队
//                // m_fftQueue.enqueue(sample);
//            }
//        } else {
//            // --- 修正后的 float 处理 ---
//            for (int i = 0; i < n; ++i) {
//                float binValue;
//                // 【安全拷贝】即使地址是 0x1003，memcpy 也能安全读出 4 个字节的 float
//                memcpy(&binValue, payload + i * sizeof(float), sizeof(float));

//                // 调试用：打印第一个值看看对不对
//                // if (i == 0) qDebug() << "First Bin:" << binValue;

//                // TODO: 将 binValue 入队处理
//                // 注意：不要在这里直接做复杂的 UI 更新，只做数据入队
//            }
//        }
}

QByteArray DeviceAcquisitionWorker::buildStartFrame() const
{
    return QByteArray::fromHex("AA001700010D0A");
}

QByteArray DeviceAcquisitionWorker::buildStopFrame() const
{
    return QByteArray::fromHex("AA001700000D0A");
}
