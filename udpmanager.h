#ifndef UDPMANAGER_H
#define UDPMANAGER_H

#include <QObject>
#include <QUdpSocket>
#include <QHostAddress>
#include <QByteArray>
#include <QDebug>

/**
 * @brief UdpManager
 *
 * 上位机与下位机之间的 UDP 通信管理类。
 * 主要职责：
 *   1. 管理本地 UDP Socket 的创建、绑定、监听。
 *   2. 统一发送/接收数据接口。
 *   3. 提供会话开关（全局监听，但逻辑上可控）。
 *   4. 可选的白名单机制，用于限制接收数据来源（提高安全性）。
 *
 * 使用场景：
 *   - 程序启动时调用 startListening() 完成一次 bind。
 *   - 会话开始时调用 setSessionActive(true)，允许数据处理。
 *   - 会话停止时调用 setSessionActive(false)，丢弃后续数据。
 *   - 如需彻底断开 readyRead，可以调用 stopListening(true)。
 */

class UdpManager : public QObject
{
    Q_OBJECT
public:
    explicit UdpManager(QObject *parent = nullptr);
    ~UdpManager();

    // 设置本地监听地址和端口（bind 使用）
    void setLocal(const QHostAddress& ip, quint16 port);

    // 设置对端（下位机）地址和端口（sendData 使用）
    void setPeer(const QHostAddress& ip, quint16 port);

    // 设置允许接收的来源 IP 白名单（若为空则不限制）
    void setPeerWhitelist(const QList<QHostAddress>& peers);

    // 启动监听：仅第一次 bind 并连接 readyRead 信号
    bool startListening();

    void armFirstPacket();   //声明要有

    /**
     * @brief 停止监听
     * @param hard
     *   - false（默认，软停）：仅改变逻辑状态，不再处理数据；
     *   - true（硬停）：断开 readyRead 信号，彻底不再触发 onReadyRead。
     */
    void stopListening(bool hard=false);

    // 设置会话开关：true=允许上抛数据；false=丢弃数据（但仍在全局监听）
    void setSessionActive(bool on);

    // 向目标设备发送数据报（使用 setPeer() 设置的 IP/端口）
    bool sendData(const QByteArray& data);

signals:
    // 监听状态信号
    void listeningStarted();  //成功启动监听
    void listeningStopped();    // 停止监听

    // 监听失败的详细错误信息
    void initFailed(const QString &errorMsg);

    // 收到合法数据报并且会话开启时，上抛数据
    void dataReceived(const QByteArray &data);

    void stopped();

private slots:
    void onReadyRead();    // Socket 的 readyRead 信号触发时调用，统一处理收包逻辑

private:
    QUdpSocket* udpSocket = nullptr;   // 构造函数里 new 一次

    // 本地端口（用于 bind）
    QHostAddress localIp_   = QHostAddress("192.168.1.10");
    quint16      localPort_ = 8080;

    // 目标端口（用于 writeDatagram）
    QHostAddress targetIp_   = QHostAddress("192.168.1.11");
    quint16      targetPort_ = 8081;

    bool isBound_ = false;             // 是否已 bind（只做一次）
    bool isListening_ = false; // 标记是否正在监听
    bool readyReadConnected_ = false;   // 是否已连接 readyRead
    bool sessionActive_ = false;       // 会话是否开启（控制是否上抛 dataReceived）

    QSet<QHostAddress> whitelist_;    // 来源白名单（非空时，只接收列表内 IP 的数据）
    quint64 idleDropCount_ = 0;        // 会话未开启时丢弃的数据包计数（可用于审计）
    bool allowFirstPacket_ = false;
};

#endif // UDPMANAGER_H
