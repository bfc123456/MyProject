#ifndef UDPMANAGER_H
#define UDPMANAGER_H

#include <QObject>
#include <QUdpSocket>
#include <QHostAddress>
#include <QByteArray>
#include <QDebug>

class UdpManager : public QObject
{
    Q_OBJECT
public:
    explicit UdpManager(QObject *parent = nullptr);
    ~UdpManager();

    // 启动UDP监听
    bool startListening(quint16 port);

    // 停止UDP监听
    void stopListening();

signals:
    // 信号：接收到数据
    void dataReceived(const QByteArray &data);

private slots:
    // 槽：处理接收到的数据
    void onReadyRead();

private:
    QUdpSocket *udpSocket;  // 用来接收UDP数据的套接字
};

#endif // UDPMANAGER_H
