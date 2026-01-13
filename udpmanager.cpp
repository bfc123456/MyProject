#include "udpmanager.h"
#include <QHostAddress>
#include <QThread>
#include <QNetworkDatagram>
#include <QElapsedTimer>

UdpManager::UdpManager(QObject *parent)
    : QObject(parent), udpSocket(new QUdpSocket(this)){

}

UdpManager::~UdpManager()
{
    stopListening();  // 析构时确保停止监听
}

/**
 * @brief 设置本地监听 IP 和端口
 */
void UdpManager::setLocal(const QHostAddress& ip, quint16 port) {
    localIp_   = ip;
    localPort_ = port;
}

/**
 * @brief 设置下位机的 IP 和端口（发送时用）
 */
void UdpManager::setPeer(const QHostAddress& ip, quint16 port) {
    targetIp_   = ip;
    targetPort_ = port;
}

/**
 * @brief 设置来源白名单（如果非空，则只接受这些 IP 的数据）
 */
void UdpManager::setPeerWhitelist(const QList<QHostAddress>& peers) {
    whitelist_.clear();
    for (const QHostAddress& ip : peers) {
        whitelist_.insert(ip);
    }
}

/**
 * @brief 启动监听（只 bind 一次，只连一次 readyRead）
 */
bool UdpManager::startListening() {
    qInfo() << "[UdpManager] startListening() enter"
            << "isBound_=" << isBound_
            << "readyReadConnected_=" << readyReadConnected_
            << "isListening_=" << isListening_
            << "localIp_=" << localIp_.toString()
            << "localPort_=" << localPort_;

    if (!udpSocket) {
        qCritical() << "[UdpManager] udpSocket is nullptr";
        return false;
    }

    // 1) 绑定（仅一次）
    if (!isBound_) {
        qInfo() << "[UdpManager] try bind ip=" << localIp_.toString()
                << " port=" << localPort_;

        udpSocket->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, 8*1024*1024);

        // 先按“指定网卡+无flags”尝试（Windows对ShareAddress很挑）
        if (!udpSocket->bind(localIp_, localPort_)) {
            qCritical() << "[UdpManager] bind(ip,port) 失败:" << udpSocket->errorString()
                        << " → fallback AnyIPv4 + Share/Reuse";

            QAbstractSocket::BindMode flags =
                QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint;

            if (!udpSocket->bind(QHostAddress::AnyIPv4, localPort_, flags)) {
                qCritical() << "[UdpManager] bind(AnyIPv4) 仍失败:" << udpSocket->errorString();
                emit initFailed(udpSocket->errorString());
                return false;
            }
        }

        isBound_ = true;
        qInfo() << "[UdpManager] bind OK, effective local="
                << udpSocket->localAddress().toString() << ":" << udpSocket->localPort();
    }

    // 2) 连接 readyRead（只连一次）
    if (!readyReadConnected_) {
        const bool ok = connect(udpSocket, &QUdpSocket::readyRead,
                                this, &UdpManager::onReadyRead,
                                Qt::UniqueConnection);
        qInfo() << "[UdpManager] connect readyRead:" << ok;
        if (!ok) return false;
        readyReadConnected_ = true;
    }

    // 3) 重复启动保护
    if (isListening_) {
        qWarning() << "[UdpManager] already listening";
        return true;
    }

    isListening_ = true;
    emit listeningStarted();
    qInfo() << "[UdpManager] listening started";
    return true;
}

/**
 * @brief 停止监听
 * @param hard = true 表示硬停（断开 readyRead 信号），彻底不再触发回调
 *               false 表示软停（逻辑状态+会话关闭），socket 仍然绑定端口
 */
void UdpManager::stopListening(bool hard)
{
    if (hard && readyReadConnected_) {
            disconnect(udpSocket, &QUdpSocket::readyRead,
                       this, &UdpManager::onReadyRead);
            readyReadConnected_ = false;
        }

        isListening_   = false;
        sessionActive_ = false;  // 会话也随之关闭
        emit listeningStopped();

        qInfo() << "[UdpManager] 已停止监听 (hard=" << hard << ")";
}

/**
 * @brief 收到数据时的回调
 * - 白名单过滤
 * - 会话未开启时直接丢弃（仅计数）
 * - 会话开启时通过信号上抛数据
 */
void UdpManager::onReadyRead()
{
    // 统计：限频打印用
    static quint64 rxTotal = 0;
    static quint64 rxPassed = 0;
    static quint64 rxDroppedInactive = 0;
    static quint64 rxDroppedWhitelist = 0;
    static quint64 rxDroppedSize = 0;

    // 每次 readyRead 最多处理的包数，防止某些极端情况下饿死事件循环
    const int kMaxPerSignal = 512;

    int processed = 0;
    while (udpSocket->hasPendingDatagrams() && processed < kMaxPerSignal) {
        ++processed;

        QNetworkDatagram dg = udpSocket->receiveDatagram();
        const QByteArray data = dg.data();
        const QHostAddress srcIp = dg.senderAddress();
        const quint16 srcPort = dg.senderPort();

        ++rxTotal;

        // 1) 会话未开启：也要“读出来”，但不往上抛
        if (!sessionActive_) {
            ++rxDroppedInactive;
            continue;
        }

        // 2) 白名单过滤（仅当 whitelist 非空时生效）
        if (!whitelist_.isEmpty() && !whitelist_.contains(srcIp)) {
            ++rxDroppedWhitelist;
            continue;
        }

        // 3) 包长过滤（可选：按你的协议）
        // 你之前用 1472 / 492，这里建议放在 UdpManager 只做基本保护
        if (data.isEmpty()) {
            ++rxDroppedSize;
            continue;
        }

        ++rxPassed;

        // ⚠️ 不在这里做解析/FFT/数据库等重活，只转发
        emit dataReceived(data);
    }

    // 4) 限频日志：每秒打印一次即可
    static QElapsedTimer t;
    if (!t.isValid()) t.start();
    if (t.elapsed() >= 1000) {
        qInfo() << "[UdpManager] RX stats"
                << "total=" << rxTotal
                << "passed=" << rxPassed
                << "drop(inactive)=" << rxDroppedInactive
                << "drop(whitelist)=" << rxDroppedWhitelist
                << "drop(size)=" << rxDroppedSize
                << "thread=" << QThread::currentThreadId();
        t.restart();
    }

    // 5) 如果单次 readyRead 有很多包，下次事件循环会再进来处理剩余的
    // 不要在这里递归调用 onReadyRead()，让事件循环自然调度
}

/**
 * @brief 向下位机发送数据
 */
bool UdpManager::sendData(const QByteArray &data)
{
    if (!udpSocket) {
        qCritical() << "[UdpManager] sendData: udpSocket is null";
        return false;
    }

    // 关键：发之前把目标、长度、内容（前16字节）打印出来
    qInfo() << "[UdpManager] TX thread=" << QThread::currentThreadId()
            << "to=" << targetIp_.toString() << ":" << targetPort_
            << "len=" << data.size()
            << "hex(head16)=" << data.left(16).toHex(' ')
            << "hex(all)=" << data.toHex(' ');

    qint64 n = udpSocket->writeDatagram(data, targetIp_, targetPort_);

    // 关键：把 writeDatagram 的返回值和错误码一起打印
    qInfo() << "[UdpManager] writeDatagram returned =" << n
            << "socketErr=" << udpSocket->error()
            << udpSocket->errorString();

    if (n < 0) {
        qWarning() << "[UdpManager] 发送失败:" << udpSocket->errorString();
        return false;
    }

    // 关键：UDP 写成功也要确认写出的字节数是否等于 data.size()
    if (n != data.size()) {
        qWarning() << "[UdpManager] 发送字节数不完整, expected=" << data.size()
                   << "actual=" << n;
        // 这里你可以选择返回 false 或 true；我建议返回 false 更严格
        return false;
    }

    return true;
}


void UdpManager::armFirstPacket() {
    allowFirstPacket_ = true;
}

void UdpManager::setSessionActive(bool on)
{
    sessionActive_ = on;
}
