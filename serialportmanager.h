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

//    bool openPort(const QString &portName, qint32 baudRate = QSerialPort::Baud115200);
//    void closePort();
//    bool sendCommand(const QByteArray &command);

    // 射频板卡控制指令
    void sendAttenuationCommand(uint8_t attenA, uint8_t attenB, uint8_t attenC);
    void parseAttenuationResponse(QByteArray data);
    void sendReceiverStatusQuery();
    void parseReceiverStatusResponse(QByteArray data);
    void sendTransmitterStatusQuery();
    void parseTransmitterStatusResponse(QByteArray data);


signals:
    void dataReceived(QByteArray data);
    void errorOccurred(QString errorMsg);

private slots:
    void readData();

private:
    QSerialPort *serialPort;
};

#endif // SERIALPORTMANAGER_H
