#ifndef DEVICEACQUISITIONWORKER_H
#define DEVICEACQUISITIONWORKER_H

#include <QObject>
#include <QMutex>
#include <QTimer>
#include <QQueue>
#include "UdpManager.h"
#include "MedicalLogger.h"

/**
 * @class DeviceAcquisitionWorker
 * @brief 设备采集工作线程对象
 *
 * 本类运行在接收线程中，负责管理与下位机（设备）的 UDP 通信，
 * 包括初始化网络、发送会话控制命令、接收并解析数据。
 *
 * 功能职责：
 * - 初始化 UdpManager 并绑定到接收线程
 * - 管理采集会话的启停（发送 Start / Stop 命令）
 * - 控制会话状态：等待首包 / 确认 ACK 后才进入采集状态
 * - 处理下位机上报的 UDP 数据，并解析出 mag5 数值上抛
 * - 在采集会话的不同阶段发出信号（sessionStarted、acquisitionStopped、acquisitionError）
 *
 * 特点与设计：
 * - 与 UdpManager 解耦，DeviceAcquisitionWorker 专注业务逻辑，UdpManager 专注收发数据
 * - 采用信号槽机制与 UI 层通信，确保线程安全
 * - 提供 sessionActive_ 和 awaitingFirst_ 标志位，保证会话状态的严格控制
 * - 可扩展缓存/滴灌逻辑（m_valueCache, m_cacheDrainer 等），适应高频数据流平滑处理
 *
 * 使用场景：
 * - 在应用初始化时创建并移动到接收线程
 * - 登录后或测量流程中调用 startAcquisition() 开始采集
 * - UI 根据信号更新状态（如“RUNNING”“IDLE”“ERROR”）
 *
 * @note 本类必须在 moveToThread() 后、线程启动后调用 initUdpManager()，
 *       否则无法正确完成 UDP 绑定与信号连接。
 */

class DeviceAcquisitionWorker : public QObject {
    Q_OBJECT
public:
    explicit DeviceAcquisitionWorker(QObject *parent = nullptr);
    ~DeviceAcquisitionWorker() override;

    // 固定通信参数（业务层配置）
    static const QHostAddress kLocalIp;
    static const quint16      kLocalPort;
    static const QHostAddress kDeviceIp;
    static const quint16      kDevicePort;
//    void startCacheDrainer();

public slots:
    /**
     * @brief 在接收线程中初始化网络（bind + 连接 readyRead）
     * 必须在该对象已经 moveToThread 对应线程并且线程 start() 后调用。
     */
    void initUdpManager();

    /**
     * @brief 开始一次会话：发送 Start 命令，等待首包（或 ACK）后才真正启用数据处理。
     * 注意：UdpManager 始终在监听，但只有 sessionActive_ 开启时才会上抛 dataReceived。
     */
    void startAcquisition();
    /**
     * @brief 停止会话：发送 Stop 命令并关闭会话开关。
     */
    void stopAcquisition();

signals:
    /**
     * @brief 首包（或 START_ACK）到达，确认下位机已开始发送。
     * UI 可以据此更新状态为 “RUNNING”。
     */
    void sessionStarted();

    /**
     * @brief 会话停止（软停/硬停），UI 可据此回到 IDLE。
     */
    void acquisitionStopped();

    /**
     * @brief 发生不可恢复错误（bind 失败、发送失败等）。
     */
    void acquisitionError(const QString& msg);

    /**
     * @brief 解析出一条 mag5 数据（你现有的信号，供处理器使用）。
     */
    void mag5DataReceived(quint32 currentValue);

private slots:
    /**
     * @brief 处理 UdpManager 上抛的原始 UDP 数据（仅会话开启时才会收到）。
     */
    void onReadyRead(const QByteArray& data);

//    void drainCacheTick();              // 定时从缓存中取一个发出

private:
    // 内部工具
    QByteArray buildStartFrame() const; // 构造 0x55 Start
    QByteArray buildStopFrame()  const; // 构造 0xAA Stop

private:
    UdpManager* udpManager{nullptr};
    bool sessionActive_{false};       // 会话开关（控制是否处理数据）
    bool awaitingFirst_{false};       // 已发 Start，等待首包/ACK 确认

//    QQueue<quint32> m_valueCache;       // 缓存最大值（FIFO）
//    QTimer          m_cacheDrainer;     // 滴灌定时器
//    QMutex          m_cacheMutex;       // 保护队列
//    int             m_maxCacheSize = 4096; // 缓存上限，防止堆积
//    int             m_pointsPerTick = 1;   // 每次tick发几个点（1=最平滑）
};

#endif // DEVICEACQUISITIONWORKER_H
