#include "deviceacquisitionworker.h"
#include "udpmanager.h"
#include <QDebug>
#include <QThread>
#include <QtEndian>

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
    qInfo() << "[Affinity] worker =" << this->thread()
            << "udpManager =" << udpManager->thread();
}

void DeviceAcquisitionWorker::requestStart()
{
    qInfo() << "[DeviceWorker] requestStart() thread=" << QThread::currentThreadId()
            << "state=" << int(m_state.get())
            << "udpManager=" << (void*)udpManager;

    if (!m_state.is(ThreadWorkState::Idle)) {
        qWarning() << "[DeviceWorker] abort: not Idle, state=" << int(m_state.get());
        return;
    }

    if (!udpManager) {
        qCritical() << "[DeviceWorker] abort: udpManager is null";
        emit acquisitionError("UDP not initialized");
        m_state.set(ThreadWorkState::Error);
        return;
    }

    m_state.set(ThreadWorkState::Starting);

    QByteArray frame = buildStartFrame();
    qInfo() << "[DeviceWorker] START frame=" << frame.toHex(' ');

    bool ok = udpManager->sendData(frame);
    qInfo() << "[DeviceWorker] send START ok=" << ok;

    if (!ok) {
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

void DeviceAcquisitionWorker::onUdpPacket(const QByteArray& packet) {
    // 提取当前包的计数值
    int currentCount = extractCountFromPacket(packet);

    // 如果是第一次接收到数据包，初始化 lastReceived 和 expectedCount
    if (lastReceived == -1) {
        lastReceived = 1;  // 第一个包接收时计数从1开始
        expectedCount = currentCount + 1;  // 预期值是当前包的计数 + 1
        qDebug() << "First packet received. Expected: " << expectedCount;
        qDebug() << "RX Packet: " << packet.toHex();
        return;
    }

    // 如果接收到的包计数值与预期的不同，说明有丢包或乱序
    if (currentCount != expectedCount) {
        int lostPackets = currentCount - expectedCount;
        if (lostPackets > 0) {
            qDebug() << "Packet loss detected! Expected: " << expectedCount << " Received: " << currentCount
                     << " Lost packets: " << lostPackets;
        } else if (lostPackets < 0) {
            qDebug() << "Out of order packet received! Expected: " << expectedCount << " Received: " << currentCount;
        }
    }

    // 更新实际接收到的数据包计数
    lastReceived++;

    // 更新预期的下一个包计数
    expectedCount = currentCount + 1;  // 下一包的预期计数是当前包计数 + 1

    // 打印当前接收到的数据包
//    qDebug() << "RX Packet: " << packet.toHex();
}


// 提取计数值的函数（假设计数值存储在包的某个位置，举例取第4个字节）
int DeviceAcquisitionWorker::extractCountFromPacket(const QByteArray& packet) {
    // 假设计数值在第4个字节（根据协议定义）
    if (packet.size() >= 5) {
        return static_cast<unsigned char>(packet[3]);  // 取第4个字节作为计数值
    }
    return -1;  // 无效值，表示数据包格式不正确
}


QByteArray DeviceAcquisitionWorker::buildStopFrame() const
{
    return QByteArray::fromHex("AA001700000D0A");
}

QByteArray DeviceAcquisitionWorker::buildStartFrame() const
{
    return QByteArray::fromHex("AA001700010D0A");
}
