#include "udpsender.h"

UdpSender::UdpSender(QObject *parent) : QObject(parent), targetAddress("192.168.1.11"), targetPort(8080) {}

void UdpSender::sendFrequencyCommand(quint32 frequencyHz) {
    QByteArray cmd;
    cmd.append(QByteArray::fromHex("0000010002000A350001020000000000000100"));

    for (int i = 3; i >= 0; --i)
        cmd.append(static_cast<char>((frequencyHz >> (i * 8)) & 0xFF));

    cmd.append(QByteArray::fromHex("00000000"));
    udpSocket.writeDatagram(cmd, targetAddress, targetPort);
}

void UdpSender::sendStartCommand() {
    QByteArray cmd = QByteArray::fromHex("0000010002000A350001020000000000000100");
    udpSocket.writeDatagram(cmd, targetAddress, targetPort);
}

