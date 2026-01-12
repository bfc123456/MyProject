#ifndef DEVICEACQUISITIONWORKER_H
#define DEVICEACQUISITIONWORKER_H

#include <QObject>
#include <QMutex>
#include <QTimer>
#include <QQueue>
#include "medicallogger.h"
#include "thread_work_state.h"
#include "measurementdataprocessor.h"

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

class UdpManager;

class DeviceAcquisitionWorker : public QObject
{
    Q_OBJECT
public:
    explicit DeviceAcquisitionWorker(QObject *parent = nullptr);
    ~DeviceAcquisitionWorker();

public slots:
    void initUdpManager();
    void requestStart();    // UI点击开始：发送START + 允许分发
    void requestStop();     // UI点击停止：发送STOP + 禁止分发

private slots:
    void onUdpPacket(const QByteArray &data);
signals:
    void rawPacketReceived(const QByteArray& data);
    void fftPacketReceived(const QByteArray& data);
    void acquisitionStopped();
    void acquisitionError(const QString& msg);

private:
    QByteArray buildStartFrame() const;
    QByteArray buildStopFrame() const;

private:
    AtomicState m_state;
    UdpManager* udpManager = nullptr;
};


#endif // DEVICEACQUISITIONWORKER_H
