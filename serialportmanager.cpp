#include "serialportmanager.h"
#include <QDebug>

SerialPortManager::SerialPortManager(QObject *parent) : QObject(parent), serialPort(new QSerialPort(this))
{
    // 连接 readyRead 信号到槽函数
    connect(serialPort, &QSerialPort::readyRead, this, &SerialPortManager::onReadyRead);
}

SerialPortManager::~SerialPortManager()
{
    if (serialPort->isOpen()) {
        serialPort->close();
    }
}

bool SerialPortManager::openPort(const QString &portName)
{
    serialPort->setPortName(portName);
    serialPort->setBaudRate(QSerialPort::Baud115200);
    serialPort->setDataBits(QSerialPort::Data8);
    serialPort->setParity(QSerialPort::NoParity);
    serialPort->setStopBits(QSerialPort::OneStop);
    serialPort->setFlowControl(QSerialPort::NoFlowControl);

    if (serialPort->open(QIODevice::ReadWrite)) {
        qDebug() << "串口打开成功:" << portName;
        return true;
    } else {
        qDebug() << "串口打开失败：" << serialPort->errorString();
        return false;
    }
}

void SerialPortManager::closePort()
{
    if (serialPort->isOpen()) {
        serialPort->close();
        qDebug() << "串口已关闭";
    }
}

void SerialPortManager::sendData(const QByteArray &data)
{
    if (serialPort->isOpen()) {
        serialPort->write(data);
        qDebug() << "发送数据：" << data.toHex();
    }
}

QByteArray SerialPortManager::readData()
{
    return serialPort->readAll();
}

void SerialPortManager::onReadyRead()
{
    QByteArray data = readData();
    qDebug() << "接收数据：" << data.toHex();
    emit dataReceived(data);  // 发出数据接收信号
}
