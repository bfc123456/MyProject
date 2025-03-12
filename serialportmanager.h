#ifndef SERIALPORTMANAGER_H
#define SERIALPORTMANAGER_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>

class SerialPortManager : public QObject
{
    Q_OBJECT

public:
    explicit SerialPortManager(QObject *parent = nullptr);
    ~SerialPortManager();

    bool openPort(const QString &portName);  // 打开串口
    void closePort();  // 关闭串口
    void sendData(const QByteArray &data);  // 发送数据
    QByteArray readData();  // 读取数据

signals:
    void dataReceived(const QByteArray &data);  // 数据接收信号

private slots:
    void onReadyRead();  // 串口数据准备好时的槽函数

private:
    QSerialPort *serialPort;  // 串口对象
};

#endif // SERIALPORTMANAGER_H
