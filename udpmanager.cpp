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
        // 1. 必须先绑定！绑定后 Socket 才有真实的系统资源
        if (!udpSocket_->bind(localIp_, localPort_)) {
            emit initFailed(udpSocket_->errorString());
            return false;
        }

        // 2. 绑定成功后，立即拉大内核接收缓冲区
        // 5MB/s 速率下，建议直接给到 128MB，对抗启动时的系统调度延迟
        udpSocket_->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption,
                                    128 * 1024 * 1024);

        // 3. 微小延时，确保操作系统完成非分页内存的分配
        QThread::msleep(50);

        // 4. 读取真实分配值进行验证
        int actualBuf = udpSocket_->socketOption(QAbstractSocket::ReceiveBufferSizeSocketOption).toInt();
        qInfo() << "[Performance Check] 系统真正分配的内核缓冲区大小:"
                << (actualBuf / 1024 / 1024) << "MB";

        isBound_ = true;
        qInfo() << "[UdpManager] bind OK"
                << udpSocket_->localAddress().toString() << ":" << udpSocket_->localPort()
                << "thread=" << QThread::currentThreadId();
    }

    if (!readyReadConnected_) {
        // 使用 UniqueConnection 防止重复连接
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

    Q_ASSERT(QThread::currentThread() == this->thread());

    if (!udpSocket_) {
        return false;
    }

    // 3. 执行发送
    const qint64 n = udpSocket_->writeDatagram(data, targetIp_, targetPort_);

    // 4. 打印结果日志
    // 获取错误信息
    QString errStr = udpSocket_->errorString();

    return n == data.size();
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
    // 1. 预分配固定大小的栈空间或预分配 QByteArray
    static thread_local char buffer[2048];

    while (udpSocket_->hasPendingDatagrams()) {
        qint64 sz = udpSocket_->pendingDatagramSize();
        if (sz <= 0) break;

        // 2. 直接读取到预分配内存，避免 resize 的内存分配开销
        qint64 rd = udpSocket_->readDatagram(buffer, sizeof(buffer));
        if (rd <= 0) continue;

        m_rawInputCount++; // 【新增】统计进入应用的物理包数

        if (sessionActive_) {
            // 3. 只有入队这一个动作，尽量减少锁竞争
            QByteArray data(buffer, int(rd));
            if (!pushPacket(std::move(data))) {
                m_queueDropCount++; // 【新增】统计入队失败
            }
        }
    }
}
