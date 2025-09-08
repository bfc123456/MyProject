// UdpSender.h
#ifndef UDPSENDER_H
#define UDPSENDER_H

#include <QObject>
#include <QUdpSocket>

class UdpSender : public QObject {
    Q_OBJECT
public:
    explicit UdpSender(QObject *parent = nullptr);
    void sendFrequencyCommand(quint32 frequencyHz);
    void sendStartCommand();

private:
    QUdpSocket udpSocket;
    QHostAddress targetAddress;
    quint16 targetPort;
};


#endif // UDPSENDER_H
