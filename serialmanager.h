#ifndef SERIALMANAGER_H
#define SERIALMANAGER_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>

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
