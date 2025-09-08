#include "DeviceAcquisitionWorker.h"
#include <QHostAddress>
#include <QDebug>
#include <QByteArray>
#include <QtEndian>
#include <QThread>
#include <cstring>

DeviceAcquisitionWorker::DeviceAcquisitionWorker(QObject *parent)
    : QObject(parent)
{

    //不在构造函数中创建udpManager！
    qDebug() << "[DeviceAcquisitionWorker] 构造函数（主线程ID：" << QThread::currentThreadId() << "）";
}

DeviceAcquisitionWorker::~DeviceAcquisitionWorker() {
    if (udpManager) {
        udpManager->deleteLater(); // 让它在自己的线程安全销毁
        udpManager = nullptr;
    }
}

//void DeviceAcquisitionWorker::startCacheDrainer() {

//    // 定时器配置，但不启动
//    m_cacheDrainer.setInterval(1);              // 默认 1ms
//    m_cacheDrainer.setTimerType(Qt::PreciseTimer);

//    connect(&m_cacheDrainer, &QTimer::timeout,
//            this, &DeviceAcquisitionWorker::drainCacheTick,
//            Qt::QueuedConnection);
//}

/*
 * @
*/
void DeviceAcquisitionWorker::initUdpManager() {

    if (udpManager) {
        qWarning() << "[Worker] udpManager 已存在，不重复初始化";
        return;
    }

    udpManager = new UdpManager(this);  //只创建一次

    // 绑定 + 监听（只做一次）
   if (!udpManager->startListening()) {
       emit acquisitionError("UDP bind failed");
       return;
   }

   // 只有会话开启时，UdpManager 才会 emit dataReceived 到这里
   connect(udpManager, &UdpManager::dataReceived,
           this,        &DeviceAcquisitionWorker::onReadyRead,
           Qt::QueuedConnection);

   connect(udpManager, &UdpManager::listeningStopped,
           this,        &DeviceAcquisitionWorker::acquisitionStopped,
           Qt::QueuedConnection);

   qDebug() << "[Worker] UdpManager init ok, thread=" << QThread::currentThreadId();
}

/**
 * @brief 发送 Start 命令：
 *  - 先关闭会话（避免旧会话残留）
 *  - 发送 0x55
 *  - 标记“等待首包/ACK”
 *  - 暂时不打开 sessionActive_（等确认后再打开）
 */
void DeviceAcquisitionWorker::startAcquisition() {
    if (!udpManager) { emit acquisitionError("UdpManager 未初始化"); return; }
    // 先停滴灌器，确保干净起步
//    QMetaObject::invokeMethod(&m_cacheDrainer, "stop", Qt::QueuedConnection);

//    //无条件清缓存，避免上次残留
//    {
//        QMutexLocker lk(&m_cacheMutex);
//        m_valueCache.clear();
//    }

    // 重置会话状态
    sessionActive_ = false;
    awaitingFirst_ = true;
    udpManager->setSessionActive(false); // 先不处理数据（防串包）
    udpManager->armFirstPacket();    // 让 UdpManager 只放行首包

    // 发送 Start 帧（按你的设备协议，这里用 0x55）
    const QByteArray startFrame = buildStartFrame();
    if (!udpManager->sendData(startFrame)) {
        emit acquisitionError("发送 START 失败");
        awaitingFirst_ = false;
        return;
    }

    qInfo() << "[Worker] START 已发送，等待首包/ACK...";
}

/**
 * @brief 发送 Stop 命令并关闭会话。
 */
void DeviceAcquisitionWorker::stopAcquisition() {
    if (!udpManager) return;

    // 发送 Stop 帧（按你的设备协议，这里用 0xAA）
    const QByteArray stopFrame = buildStopFrame();
    udpManager->sendData(stopFrame); // 失败也无所谓，反正我们本地会关

    // 关闭会话开关（UdpManager 仍在全局监听）
    sessionActive_ = false;
    awaitingFirst_ = false;
    udpManager->setSessionActive(false);

//    QMetaObject::invokeMethod(&m_cacheDrainer, "stop", Qt::QueuedConnection);

//    // 清掉残留缓存
//    {
//        QMutexLocker lk(&m_cacheMutex);
//        m_valueCache.clear();
//    }


    qInfo() << "[Worker] 会话停止（软停）";
    emit acquisitionStopped();
}

/**
 * @brief 仅在会话开启时才会被 UdpManager 调用（dataReceived 上抛）。
 * 这里解析 1456 字节包、切 28 组、取 offset=20 的 4 字节作为 mag5。
 * 首包到达时（awaitingFirst_==true）会触发 sessionActive_=true 并通知 UI。
 */
void DeviceAcquisitionWorker::onReadyRead(const QByteArray &data) {
    //1. 首包确认：第一次收到数据才开启会话
    if (awaitingFirst_) {
        awaitingFirst_ = false;
        sessionActive_ = true;
        if (udpManager) udpManager->setSessionActive(true);
        udpManager->setSessionActive(true);   // 打开 gate
//        QMetaObject::invokeMethod(&m_cacheDrainer, "start", Qt::QueuedConnection);// 🔹此时才启动滴灌器
        emit sessionStarted();
        qInfo() << "[Worker] 首包到达，确认会话已开始";
    }

    // 会话外直接返回（全局监听但逻辑可控）
    if (!sessionActive_) return;

    // ====== 2) 固定参数（与协议一致） ======
    constexpr int  kDatagramBytes  = 1456;          // 整包字节数
    constexpr int  kWordBytes      = 4;       // 一个数据=4字节
    constexpr int  kWordsPerFrame   = kDatagramBytes / kWordBytes; // 364
    constexpr int  kGroupSize       = 13;     // 每 13 个数据成组取最大
    constexpr bool kStrictHeader32  = true;   // true: 头必须等于 0x00000055
    constexpr bool kLittleEndian    = true;   // 0x55000000 → 小端

    if (data.size() != kDatagramBytes) {
        qWarning() << "[Worker] 长度异常 exp=" << kDatagramBytes
                   << " got=" << data.size();
        return;
    }

    const uchar* base = reinterpret_cast<const uchar*>(data.constData());

       // 小工具：按端序把 4 字节转为主机端 uint32
       auto load_u32 = [&](const uchar* p) -> quint32 {
           quint32 v = 0;
           std::memcpy(&v, p, kWordBytes);
           return kLittleEndian ? qFromLittleEndian(v) : qFromBigEndian(v);
       };

       // ===== 2) 解析帧头 =====
       const quint32 header = load_u32(base);
       bool headerOk = kStrictHeader32 ? (header == 0x00000055u)
                                       : ((header & 0xFFu) == 0x55u);

       if (!headerOk) {
           QByteArray first8(reinterpret_cast<const char*>(base), 8);
           qWarning() << "[Worker] 帧头异常 raw8=" << first8.toHex();
           return; // 安全：整帧丢弃
       }

       // 4) 跳过帧头，按 13 合 1 逐点发送（尾部 12 个丢弃，不跨包）
           const uchar* p = base + kWordBytes;       // 第二个 u32 起
           const int remainWords = kWordsPerFrame - 1;     // 363
           const int fullGroups  = remainWords / kGroupSize; // 27

           for (int g = 0; g < fullGroups; ++g) {
               const uchar* grp = p + g * kGroupSize * kWordBytes;

               quint32 mx = 0; bool init = false;
               for (int i = 0; i < kGroupSize; ++i) {
                   quint32 v = load_u32(grp + i * kWordBytes);
                   if (!init || v > mx) { mx = v; init = true; }
               }
               emit mag5DataReceived(mx);  // ☆ 逐个发给处理线程（QueuedConnection）
           }
}

QByteArray DeviceAcquisitionWorker::buildStartFrame() const {
    QByteArray b;
    b.append(char(0x55));   // 按协议替换
    return b;
}

QByteArray DeviceAcquisitionWorker::buildStopFrame() const {
    QByteArray b;
    b.append(char(0xAA));   // 按协议替换
    return b;
}

