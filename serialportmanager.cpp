#include "serialportmanager.h"
#include "mainwindow.h"
#include <QDebug>

SerialPortManager::SerialPortManager(QObject *parent) : QObject(parent), serialPort(new QSerialPort(this))
{
    connect(serialPort, &QSerialPort::readyRead, this, &SerialPortManager::readData);
    connect(serialPort, &QSerialPort::errorOccurred, this, [this](QSerialPort::SerialPortError error){
        if (error != QSerialPort::NoError)
            emit errorOccurred(serialPort->errorString());
    });
}

SerialPortManager::~SerialPortManager()
{
    closePort();
}

bool SerialPortManager::openPort(const QString &portName, qint32 baudRate)
{
    if (serialPort->isOpen()) {
        serialPort->close();
    }

    serialPort->setPortName(portName);
    serialPort->setBaudRate(baudRate);
    serialPort->setDataBits(QSerialPort::Data8);
    serialPort->setParity(QSerialPort::NoParity);
    serialPort->setStopBits(QSerialPort::OneStop);
    serialPort->setFlowControl(QSerialPort::NoFlowControl);

    if (!serialPort->open(QIODevice::ReadWrite)) {
        emit errorOccurred(serialPort->errorString());
        return false;
    }

    qDebug() << "RS-485 Serial Port Opened: " << portName;
    return true;
}

void SerialPortManager::closePort()
{
    if (serialPort->isOpen()) {
        serialPort->close();
        qDebug() << "RS-485 Serial Port Closed";
    }
}

bool SerialPortManager::sendCommand(const QByteArray &command)
{
    if (!serialPort->isOpen()) {
        emit errorOccurred("Serial port is not open");
        return false;
    }

    qint64 bytesWritten = serialPort->write(command);
    if (bytesWritten == -1) {
        emit errorOccurred("Failed to write data");
        return false;
    }

    return true;
}

void SerialPortManager::readData()
{
    QByteArray data = serialPort->readAll();
    qDebug() << "RS-485 Data Received: " << data.toHex();
    emit dataReceived(data);
}


//发送 接收通道衰减控制指令
void SerialPortManager::sendAttenuationCommand(uint8_t attenA, uint8_t attenB, uint8_t attenC)
{
    QByteArray command;
    command.append(0x55);  // 帧头
    command.append(0xAA);  // 指令类型
    command.append(attenA); // 衰减器A（0-30dB）
    command.append(attenB); // 衰减器B（0-30dB）
    command.append(attenC); // 衰减器C（0-30dB）

    // 计算校验和（0~2字节累加和，不计溢出）
    uint8_t checksum = (0x55 + 0xAA + attenA + attenB + attenC) & 0xFF;
    command.append(checksum);

    serialPort->write(command);  // 发送指令
    qDebug() << "[Sent] Attenuation Command: " << command.toHex();
}

//解析 接收通道衰减设置应答指令
void SerialPortManager::parseAttenuationResponse(QByteArray data)
{
    if (data.size() < 4) return;

    uint8_t header = data[0];
    uint8_t commandType = data[1];
    uint8_t result = data[2]; // 设置结果
    uint8_t checksum = data[3];

    uint8_t calculatedChecksum = (header + commandType + result) & 0xFF;
    if (header != 0x55 || calculatedChecksum != checksum) {
        qDebug() << "Attenuation Response: Checksum Error!";
        return;
    }

    if (result == 0x00) {
        qDebug() << "Attenuation setting success!";
    } else {
        qDebug() << "Attenuation setting failed!";
    }
}

//发送 接收通道状态查询指令
void SerialPortManager::sendReceiverStatusQuery()
{
    QByteArray command;
    command.append(static_cast<quint8>(0x55));  // 帧头
    command.append(static_cast<quint8>(0xBB));  // 指令类型
    command.append(static_cast<quint8>(0x00));  // 模块编号（0 = 接收通道）

    uint8_t checksum = (0x55 + 0xBB + 0x00) & 0xFF;
    command.append(static_cast<quint8>(checksum));

    serialPort->write(command);  // 发送指令
    qDebug() << "[Sent] Receiver Status Query: " << command.toHex();
}

//解析 接收通道状态查询应答指令
void SerialPortManager::parseReceiverStatusResponse(QByteArray data)
{
    if (data.size() < 4) return;

    uint8_t header = data[0];
    uint8_t commandType = data[1];
    uint8_t attenuation = data[2]; // 衰减器A的衰减值
    uint8_t checksum = data[3];

    uint8_t calculatedChecksum = (header + commandType + attenuation) & 0xFF;
    if (header != 0x55 || calculatedChecksum != checksum) {
        qDebug() << "Receiver Status: Checksum Error!";
        return;
    }

    qDebug() << "Receiver Attenuation: " << attenuation << " dB";
//    diastolicValue->setText(QString::number(attenuation) + " dB");
}

//发送 发射通道状态查询指令
void SerialPortManager::sendTransmitterStatusQuery()
{
    QByteArray command;
    command.append(0x55);  // 帧头
    command.append(0xBB);  // 指令类型
    command.append(0x01);  // 模块编号（1 = 发射通道）

    uint8_t checksum = (0x55 + 0xBB + 0x01) & 0xFF;
    command.append(checksum);

    serialPort->write(command);  // 发送指令
    qDebug() << "[Sent] Transmitter Status Query: " << command.toHex();
}

//解析 发射通道状态查询应答指令
void SerialPortManager::parseTransmitterStatusResponse(QByteArray data)
{
    if (data.size() < 8) return;

    uint8_t header = data[0];
    uint8_t commandType = data[1];
    uint8_t voltage = data[2];
    uint8_t current = data[3];
    uint8_t temperature = data[4];
    uint8_t power = data[5];
    uint8_t vswr = data[6];
    uint8_t checksum = data[7];

    uint8_t calculatedChecksum = (header + commandType + voltage + current + temperature + power + vswr) & 0xFF;
    if (header != 0x55 || calculatedChecksum != checksum) {
        qDebug() << "Transmitter Status: Checksum Error!";
        return;
    }

    qDebug() << "Transmitter Status - Voltage:" << (voltage == 0 ? "Normal" : "Fault");
    qDebug() << "Transmitter Status - Current:" << (current == 0 ? "Normal" : "Fault");
    qDebug() << "Transmitter Status - Temperature:" << (temperature == 0 ? "Normal" : "Fault");
    qDebug() << "Transmitter Status - Output Power:" << (power == 0 ? "Normal" : "Fault");
    qDebug() << "Transmitter Status - VSWR:" << (vswr == 0 ? "Normal" : "Fault");

//    heartbeatValue->setText(voltage == 0 ? "正常" : "故障");
}



