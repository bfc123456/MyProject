#include "udpmanager.h"

UdpManager::UdpManager(QObject* parent)
    : QObject(parent),
      udpSocket_(new QUdpSocket(this))
{
    // 预分配队列容量：避免运行时扩容
    ring_.resize(kCap);
    qInfo() << "[DEBUG] RingBuffer Size:" << ring_.size() << " Capacity:" << ring_.capacity();

    targetIp_ = QHostAddress("192.168.1.11");
    targetPort_ = 8081;

    qInfo() << "[UdpManager] Init OK. Target:" << targetIp_ << ":" << targetPort_;
}

UdpManager::~UdpManager()
{
    stopListening(true);
}

bool UdpManager::startListening()
{
    if (!udpSocket_) return false;

    if (!isBound_) {
        // 尽可能拉大接收缓冲（Windows 可能被上限截断，但设置更大通常仍有收益）
        udpSocket_->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption,
                                    64 * 1024 * 1024); // 64MB

        // 绑定
        if (!udpSocket_->bind(localIp_, localPort_)) {
            emit initFailed(udpSocket_->errorString());
            return false;
        }

        isBound_ = true;
        qInfo() << "[UdpManager] bind OK"
                << udpSocket_->localAddress().toString() << ":" << udpSocket_->localPort()
                << "thread=" << QThread::currentThreadId();
    }

    if (!readyReadConnected_) {
        bool ok = connect(udpSocket_, &QUdpSocket::readyRead,
                          this, &UdpManager::onReadyRead,
                          Qt::UniqueConnection);
        if (!ok) return false;
        readyReadConnected_ = true;
    }

    if (isListening_) return true;

    isListening_ = true;
    emit listeningStarted();
    qInfo() << "[UdpManager] listening started";
    return true;
}

void UdpManager::stopListening(bool hard)
{
    if (hard && readyReadConnected_) {
        disconnect(udpSocket_, &QUdpSocket::readyRead,
                   this, &UdpManager::onReadyRead);
        readyReadConnected_ = false;
    }

    isListening_ = false;
    sessionActive_ = false;
    emit listeningStopped();
    qInfo() << "[UdpManager] listening stopped (hard=" << hard << ")";
}

void UdpManager::setSessionActive(bool on)
{
    sessionActive_ = on;
}

bool UdpManager::sendData(const QByteArray& data)
{
    if (!udpSocket_) return false;
    qint64 n = udpSocket_->writeDatagram(data, targetIp_, targetPort_);
    qInfo() << "[DEBUG] Sending to:" << targetIp_ << ":" << targetPort_;
    return (n == data.size());
}

uint32_t UdpManager::queueSize() const
{
    uint32_t h = head_.load(std::memory_order_acquire);
    uint32_t t = tail_.load(std::memory_order_acquire);
    return (t - h);
}

// producer (onReadyRead) only
bool UdpManager::pushPacket(QByteArray&& pkt)
{
    uint32_t t = tail_.load(std::memory_order_relaxed);
    uint32_t h = head_.load(std::memory_order_acquire);

    // 满：丢弃（应用层丢）
    if ((t - h) >= kCap) {
        droppedByQueue_.fetch_add(1, std::memory_order_relaxed);
        return false;
    }

    ring_[t & kMask] = std::move(pkt);
    tail_.store(t + 1, std::memory_order_release);
    return true;
}

// consumer (DeviceWorker) only
int UdpManager::popBatch(QVector<QByteArray>& out, int maxBatch)
{
    uint32_t h = head_.load(std::memory_order_relaxed);
    uint32_t t = tail_.load(std::memory_order_acquire);

    uint32_t avail = (t - h);
    if (avail == 0) return 0;

    uint32_t n = qMin<uint32_t>(uint32_t(maxBatch), avail);

    out.reserve(out.size() + int(n));
    for (uint32_t i = 0; i < n; ++i) {
        out.push_back(std::move(ring_[(h + i) & kMask]));
    }

    head_.store(h + n, std::memory_order_release);
    return int(n);
}

void UdpManager::onReadyRead()
{
    // 用 thread_local 复用丢弃包的缓存，避免每个丢弃包都 new QByteArray
    static thread_local QByteArray trash;

    while (udpSocket_->hasPendingDatagrams()) {
        const qint64 sz = udpSocket_->pendingDatagramSize();
        if (sz <= 0) break;

        QHostAddress srcIp;
        quint16 srcPort = 0;

        // 先用 trash 读一遍拿到 srcIp/srcPort？——不行，readDatagram 读完就没了
        // 所以策略是：先读到 trash，再根据过滤条件决定是否“转正”为入队数据。
        trash.resize(int(sz));                 // 这一步不会每次重新分配（容量够就复用）
        const qint64 rd = udpSocket_->readDatagram(trash.data(), trash.size(), &srcIp, &srcPort);
        if (rd <= 0) continue;

        rxTotal_++;
        rxBytesTotal_ += (quint64)rd;

        // 会话未开启：读空但不入队
        if (!sessionActive_) { rxDropInactive_++; continue; }

        // 固定对端过滤
        if (srcIp != targetIp_ || srcPort != targetPort_) { rxDropPeer_++; continue; }

        // rd 才是有效长度
        if (rd <= 0) { rxDropSize_++; continue; }

        // 只有真正要入队时，才分配“可移动”的 QByteArray
        QByteArray data(int(rd), Qt::Uninitialized);
        memcpy(data.data(), trash.constData(), size_t(rd));

        if (pushPacket(std::move(data))) {
            rxPassed_++;
            rxBytesPassed_ += (quint64)rd;
        }
        // pushPacket 失败（队列满）在 pushPacket 内部已统计 droppedByQueue_
    }

    // 每秒汇总一次
    if (!statTimer_.isValid()) statTimer_.start();
    if (statTimer_.elapsed() >= 1000) {
        const double mbTotal  = rxBytesTotal_  / (1024.0 * 1024.0);
        const double mbPassed = rxBytesPassed_ / (1024.0 * 1024.0);

        qInfo() << "[UdpManager][1s]"
                << "total=" << rxTotal_ << "(" << mbTotal << "MB/s)"
                << "passed=" << rxPassed_ << "(" << mbPassed << "MB/s)"
                << "dropInactive=" << rxDropInactive_
                << "dropPeer=" << rxDropPeer_
                << "dropSize=" << rxDropSize_
                << "dropQueue=" << droppedByQueue_.load(std::memory_order_relaxed)
                << "qSize=" << queueSize()
                << "thread=" << QThread::currentThreadId();

        rxTotal_ = rxPassed_ = rxDropInactive_ = rxDropPeer_ = rxDropSize_ = 0;
        rxBytesTotal_ = rxBytesPassed_ = 0;
        statTimer_.restart();
    }
}

