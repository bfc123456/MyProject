#include "deviceacquisitionworker.h"
#include "udpmanager.h"
#include <QDebug>
#include <QThread>

DeviceAcquisitionWorker::DeviceAcquisitionWorker(QObject *parent)
    : QObject(parent)
{
    qDebug() << "[DeviceAcquisitionWorker] ctor, thread:"
             << QThread::currentThreadId();
}

DeviceAcquisitionWorker::~DeviceAcquisitionWorker()
{
    if (udpManager) {
        udpManager->deleteLater();
        udpManager = nullptr;
    }
}

/**
 * @brief 在线程启动时调用（只一次）
 */

void DeviceAcquisitionWorker::initUdpManager()
{
    if (udpManager) {
        qWarning() << "[DeviceWorker] udpManager already exists";
        return;
    }

    udpManager = new UdpManager(this);

    if (!udpManager->startListening()) {
        emit acquisitionError("UDP bind/listen failed");
        m_state.set(ThreadWorkState::Error);
        return;
    }

    connect(udpManager, &UdpManager::dataReceived,
            this, &DeviceAcquisitionWorker::onUdpPacket,
            Qt::QueuedConnection);

    qDebug() << "[DeviceWorker] UDP ready, thread:"
             << QThread::currentThreadId();
}

void DeviceAcquisitionWorker::requestStart()
{
    if (!m_state.is(ThreadWorkState::Idle))
        return;

    m_state.set(ThreadWorkState::Starting);

    if (!udpManager) {
        emit acquisitionError("UDP not initialized");
        m_state.set(ThreadWorkState::Error);
        return;
    }

    // 发送 START
    if (!udpManager->sendData(buildStartFrame())) {
        emit acquisitionError("Send START failed");
        m_state.set(ThreadWorkState::Error);
        return;
    }

    udpManager->setSessionActive(true);
    m_state.set(ThreadWorkState::Working);

    qInfo() << "[DeviceWorker] acquisition started";
}

void DeviceAcquisitionWorker::requestStop()
{
    if (!ThreadWorkStateIsBusy(m_state.get()))
        return;

    m_state.set(ThreadWorkState::Stopping);

    if (udpManager) {
        udpManager->sendData(buildStopFrame());
        udpManager->setSessionActive(false);
    }

    m_state.set(ThreadWorkState::Idle);
    emit acquisitionStopped();

    qInfo() << "[DeviceWorker] acquisition stopped";
}

void DeviceAcquisitionWorker::onUdpPacket(const QByteArray &data)
{
    if (!m_state.is(ThreadWorkState::Working))
        return;

    constexpr int RAW_SIZE = 1472;
    constexpr int FFT_SIZE = 492;

    if (data.size() == RAW_SIZE) {
        emit rawPacketReceived(data);
    }
    else if (data.size() == FFT_SIZE) {
        emit fftPacketReceived(data);
    }
    else {
        qWarning() << "[DeviceWorker] unexpected packet size:" << data.size();
    }
}

QByteArray DeviceAcquisitionWorker::buildStartFrame() const
{
    return QByteArray(1, char(0x55));
}

QByteArray DeviceAcquisitionWorker::buildStopFrame() const
{
    return QByteArray(1, char(0xAA));
}
