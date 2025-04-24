#include "serialmanager.h"
#include <QDebug>

SerialManager::SerialManager(QObject *parent)
    : QObject(parent), serialPort(new QSerialPort(this))
{
     connect(serialPort, &QSerialPort::readyRead, this, &SerialManager::onReadyRead);
}

SerialManager::~SerialManager()
{
    if (serialPort->isOpen()) {
        serialPort->close();
    }
}

bool SerialManager::openPort(const QString &portName, int baudRate, int dataBits, int parity, int stopBits)
{
    serialPort->setPortName(portName);
    serialPort->setBaudRate(baudRate);
    serialPort->setDataBits(static_cast<QSerialPort::DataBits>(dataBits));
    serialPort->setParity(static_cast<QSerialPort::Parity>(parity));
    serialPort->setStopBits(static_cast<QSerialPort::StopBits>(stopBits));

    if (serialPort->open(QIODevice::ReadWrite)) {
        qDebug() << "串口连接成功：" << portName;
        return true;
    } else {
        qDebug() << "无法打开串口：" << portName << serialPort->errorString();
        return false;
    }
}

void SerialManager::closePort()
{
    if (serialPort->isOpen()) {
        serialPort->close();
        qDebug() << "串口已关闭";
    }
}

void SerialManager::sendData(const QByteArray &data)
{
    if (serialPort->isOpen()) {
        serialPort->write(data);
        qDebug() << "发送数据：" << data.toHex();
    } else {
        qDebug() << "串口未打开，无法发送数据";
    }
}

bool SerialManager::isPortOpen() const
{
    return serialPort->isOpen();
}

QByteArray SerialManager::readData()
{
    QByteArray data;
    if (serialPort->isOpen()) {
        if (serialPort->bytesAvailable() > 0) {
            data = serialPort->readAll();
        } else {
            qDebug() << "No data available to read.";
        }
    }
    return data;
}

void SerialManager::onReadyRead()
{
    if (serialPort->bytesAvailable()) {
        QByteArray data = serialPort->readAll();

        // 打印接收到的原始数据
        qDebug() << "接收到的数据：" << data.toHex();  // 打印为十六进制格式


//        qDebug() << "接收到的数据：" << data;

        emit dataReceived(data);  // 通过信号发送接收到的数据
    }
}



