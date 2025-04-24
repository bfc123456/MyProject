#include "udpmanager.h"
#include <QHostAddress>

UdpManager::UdpManager(QObject *parent)
    : QObject(parent), udpSocket(new QUdpSocket(this))
{
    // 连接readyRead信号到onReadyRead槽
    connect(udpSocket, &QUdpSocket::readyRead, this, &UdpManager::onReadyRead);
}

UdpManager::~UdpManager()
{
    stopListening();
}

bool UdpManager::startListening(quint16 port)
{
    // 尝试绑定UDP端口
    if (udpSocket->bind(QHostAddress::Any, port)) {
        qDebug() << "UDP监听成功，端口号：" << port;
        return true;
    } else {
        qDebug() << "无法绑定UDP端口：" << port;
        return false;
    }
}

void UdpManager::stopListening()
{
    // 停止监听并关闭套接字
    if (udpSocket->isOpen()) {
        udpSocket->close();
        qDebug() << "UDP监听已停止";
    }
}

void UdpManager::onReadyRead()
{
    // 接收数据并发出信号
    while (udpSocket->hasPendingDatagrams()) {
        QByteArray data;
        data.resize(int(udpSocket->pendingDatagramSize()));

        QHostAddress sender;
        quint16 senderPort;
        udpSocket->readDatagram(data.data(), data.size(), &sender, &senderPort);

        qDebug() << "接收到数据来自" << sender.toString() << ":" << senderPort;
        qDebug() << "数据内容：" << data.toHex();

        // 通过信号将数据发送出去
        emit dataReceived(data);
    }
}
