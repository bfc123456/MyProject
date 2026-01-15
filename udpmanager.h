#ifndef UDPMANAGER_H
#define UDPMANAGER_H

#include <QObject>
#include <QUdpSocket>
#include <QHostAddress>
#include <QElapsedTimer>
#include <QVector>
#include <QByteArray>
#include <QAbstractSocket>
#include <QThread>
#include <atomic>

class UdpManager : public QObject
{
    Q_OBJECT
public:
    explicit UdpManager(QObject* parent = nullptr);
    ~UdpManager();

    bool startListening();
    void stopListening(bool hard = false);

    void setSessionActive(bool on);

    bool sendData(const QByteArray& data);

    // consumer: DeviceWorker 线程
    int popBatch(QVector<QByteArray>& out, int maxBatch);

    // stats / monitoring
    uint64_t droppedByQueue() const { return droppedByQueue_.load(std::memory_order_relaxed); }
    uint32_t queueSize() const;

    // config
    void setLocalBind(const QHostAddress& ip, quint16 port) { localIp_ = ip; localPort_ = port; }
    void setTargetPeer(const QHostAddress& ip, quint16 port) { targetIp_ = ip; targetPort_ = port; }

signals:
    void initFailed(const QString& err);
    void listeningStarted();
    void listeningStopped();

private slots:
    void onReadyRead();

private:
    bool pushPacket(QByteArray&& pkt);

private:
    QUdpSocket* udpSocket_ = nullptr;

    QHostAddress localIp_  = QHostAddress::AnyIPv4;
    quint16      localPort_ = 8080;

    QHostAddress targetIp_ = QHostAddress("192.168.1.11"); // 按你项目默认，可改
    quint16      targetPort_ = 8081;

    bool isBound_ = false;
    bool isListening_ = false;
    bool readyReadConnected_ = false;
    bool sessionActive_ = false;

    // ===================== SPSC 无锁环形队列 =====================
    // cap 必须 2 的幂：例如 1<<20 = 1,048,576 包
    static constexpr uint32_t kCap = (1u << 20);
    static constexpr uint32_t kMask = (kCap - 1u);

    QVector<QByteArray> ring_;
    std::atomic<uint32_t> head_{0}; // consumer read index
    std::atomic<uint32_t> tail_{0}; // producer write index

    std::atomic<uint64_t> droppedByQueue_{0};

    // ===================== 统计 =====================
    QElapsedTimer statTimer_;
    uint64_t rxTotal_ = 0;
    uint64_t rxPassed_ = 0;
    uint64_t rxDropInactive_ = 0;
    uint64_t rxDropPeer_ = 0;
    uint64_t rxDropSize_ = 0;
    quint64 rxBytesTotal_ = 0;
    quint64 rxBytesPassed_ = 0;
};


#endif // UDPMANAGER_H
