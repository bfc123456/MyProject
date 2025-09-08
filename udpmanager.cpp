#include "UdpManager.h"
#include <QHostAddress>

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
    if (!udpSocket) return;

    while (udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(int(udpSocket->pendingDatagramSize()));
        QHostAddress src; quint16 sport = 0;

        udpSocket->readDatagram(datagram.data(), datagram.size(), &src, &sport);

        // 1. 白名单检查
        if (!whitelist_.isEmpty() && !whitelist_.contains(src)) {
            qWarning() << "[UdpManager] 收到非白名单 IP:" << src.toString()
                       << " 数据已丢弃";
            continue;
        }

        // 2. 会话检查
        if (!sessionActive_) {
            ++idleDropCount_; // 计数，用于审计
            if (allowFirstPacket_) {
                allowFirstPacket_ = false;    // 只放行一次
                qInfo() << "[UdpManager] 首包穿透 bytes=" << datagram.size()
                        << " from " << src.toString() << ":" << sport;
                emit dataReceived(datagram);
            }
            continue;
        }

        // 3. 会话中 → 上抛
        emit dataReceived(datagram);
    }
}

/**
 * @brief 向下位机发送数据
 */
bool UdpManager::sendData(const QByteArray &data) {
    if (!udpSocket) return false;

    qint64 n = udpSocket->writeDatagram(data, targetIp_, targetPort_);
    if (n < 0) {
        qWarning() << "[UdpManager] 发送失败:" << udpSocket->errorString();
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
