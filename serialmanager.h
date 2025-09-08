
#ifndef SERIALMANAGER_H
#define SERIALMANAGER_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>

/**
 * @class SerialManager
 * @brief 串口通信管理器
 *
 * 本类封装了对串口通信的管理，提供打开、关闭、配置和收发数据的功能，
 * 是应用程序与下位机（或外部设备）进行串口交互的核心类。
 *
 * 功能职责：
 * - 负责枚举和管理可用串口
 * - 打开/关闭指定串口，并配置参数（波特率、校验位、数据位、停止位等）
 * - 提供同步/异步数据发送接口
 * - 接收来自设备的串口数据，并通过信号槽机制上报
 * - 提供错误处理（如端口占用、设备断开）
 *
 * 使用场景：
 * - 在应用初始化时，配置并打开串口连接
 * - 在测量/维护流程中，通过串口与传感器或下位机交互
 * - 配合 SerialDebugWidget，提供可视化的调试入口
 *
 * @note
 * - 建议与 QSerialPort 结合实现（Qt 串口模块）
 * - 可与 MedicalLogger 联动，记录所有收发的数据帧，满足审计与追溯要求
 * - 和 UdpManager 类似，SerialManager 只关注通信，不关心业务逻辑
 */

class SerialManager : public QObject
{
    Q_OBJECT
public:
    explicit SerialManager(QObject *parent = nullptr);
    ~SerialManager();

    bool openPort(const QString &portName, int baudRate, int dataBits, int parity, int stopBits);    // 打开串口

    void closePort();    // 关闭串口

    void sendData(const QByteArray &data);    // 发送数据

    QByteArray readData();    // 读取串口数据

    bool isPortOpen() const;    // 串口状态


signals:

    void dataReceived(const QByteArray &data);    // 接收到数据时发出信号

private:
    QSerialPort *serialPort;

private slots:

    void onReadyRead();
};

#endif // SERIALMANAGER_H
