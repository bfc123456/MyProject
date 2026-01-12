#ifndef MEASUREMENTDIALOG_H
#define MEASUREMENTDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>
#include <QDateTime>
#include <QVector>
#include "measurementtrendwidget.h"
#include "measurementdata.h"  // 包含独立的结构体定义
#include "global.h"

//前向声明
class DeviceAcquisitionWorker;
class MeasurementDataProcessor;

/**
 * @class MeasurementDialog
 * @brief 单次测量的前端对话框（流程控制 + 进度展示 + 结果汇总）
 *
 * 本对话框用于承载一次测量的完整交互流程：从“准备就绪”到“开始测量”，
 * 再到“测量完成并查看结果”。类内维护一个简单的状态机（READY → MEASURING → COMPLETED），
 * 通过进度条与提示文本反馈测量进展，并在测量结束时汇总由后台处理器
 *（MeasurementDataProcessor）计算出的最终结果（收缩压/舒张压/平均压/心率等）。
 *
 * 典型使用：
 * 1) 构造时传入 sensorId 与初始信号强度；
 * 2) 用户点击“开始测量”→ 初始化采集与处理线程，进入 MEASURING；
 * 3) 定时刷新进度；到达总时长或收到完成信号后，转入 COMPLETED；
 * 4) 用户可点击“查看结果”或“重新测量”，并可选择提交数据库。
 *
 * 线程/模块分工（推荐）：
 * - DeviceAcquisitionWorker：采集原始数据并投递至处理线程；
 * - MeasurementDataProcessor：解析波形、峰值检测、心率与参数计算；
 * - MeasurementDialog：管理 UI 与状态、接收最终结果并展示/落库。
 *
 * 信号与槽：
 * - 槽 onStartButtonClicked()/onRestartButtonClicked()/onViewResultButtonClicked() 控制流程切换；
 * - 槽 onMeasureCompleted() 接收最终结果并更新 UI；
 * - 信号 exitOverlay()/closePatientSignalStrengthWidget() 通知上层界面跳转或收起叠层。
 *
 * 注意：
 * - 进度条刷新由定时器驱动（建议周期与 MeasurementConfig 中配置保持一致）；
 * - 数据库写入通过 updateResultToDatabase() 在 COMPLETED 后触发；
 * - 多语言支持通过 changeEvent() 动态更新界面文案。
 */

class MeasurementDialog : public QDialog
{
    Q_OBJECT
public:
    // 测量状态枚举（清晰划分流程阶段）
    enum State {
        STATE_READY,      // 信号达标，等待开始测量
        STATE_MEASURING,  // 测量正在进行
        STATE_COMPLETED   // 测量结束，可查看结果
    };

    // 构造函数：接收传感器ID和初始信号强度
    explicit MeasurementDialog(const QString &sensorId, double signalStrength, QWidget *parent = nullptr);
    ~MeasurementDialog() override;

    // 对外接口：获取当前测量结果
    const MeasurementData& getCurrentResult() const { return m_currentResult; }

public slots:
    // 接收数据处理线程的测量数据（实时更新结果）
//    void appendData(const MeasurementData& result);

protected:
    // 重写语言切换事件（支持多语言）
    void changeEvent(QEvent *event) override;

signals:
    void exitOverlay();                     // 退出当前界面（返回上一级）
    void closePatientSignalStrengthWidget(); // 关闭信号强度监测界面

private slots:
    // 核心流程控制
    void onStartButtonClicked();    // 开始测量（点击"开始测量"触发）
    void onRestartButtonClicked();  // 重新测量（点击"重新测量"触发）
    void onViewResultButtonClicked();// 查看结果（测量完成后触发）
    void updateProgress();           // 进度条刷新（定时器驱动）
    void onMeasureCompleted(const MeasurementData& finalResult);       // 测量正常结束（总时长到达触发）
    void onProcessingFinished();     // 线程停止后清理界面状态
    void onProcessingStarted();      // 线程启动后更新界面状态

    // 按钮点击事件
    void onRightButtonClicked();     // 右侧按钮（根据状态触发不同逻辑）
    void onLeftButtonClicked();      // 左侧按钮（根据状态触发不同逻辑）
    void onCancelButtonClicked();    // 关闭按钮（返回上一级）

private:
    // 界面文本更新（根据当前状态切换文案）
    void retranslateUi();
    // 线程与信号初始化（仅首次启动时执行）
    void initWorkersAndConnections();
    // 测量结果存入数据库（仅测量完成后执行）
    bool updateResultToDatabase(const MeasurementData &result);

    // 核心数据
    QString m_sensorId;                  // 传感器ID（关联数据库记录）
    double initialSignalStrength;        // 初始信号强度（界面显示用）
    MeasurementData m_currentResult;     // 存储最终测量结果（收缩压、舒张压等）

    // 状态控制
    State currentState = STATE_READY;    // 当前测量状态（默认准备中）
    bool threadsStarted = false;         // 线程是否已初始化（避免重复启动）
    bool m_isProcessing = false;         // 测量中标记（防止重复操作）
    qint64 m_measureStartTime = 0;       // 测量开始时间戳（毫秒，计算进度用）

    // UI组件（界面元素）
    QLabel *titleLabel;         // 标题栏文本（如"测量进行中"）
    QLabel *messageLabel;       // 提示信息（如"请保持位置..."）
    QProgressBar *progressBar;  // 进度条（显示测量进度）
    int progress = 0;  // 进度值，初始化为0
    QPushButton *leftButton;    // 左侧按钮（返回/重新测量）
    QPushButton *rightButton;   // 右侧按钮（开始测量/查看结果）
    QPushButton *closeButton;   // 关闭按钮（标题栏"✕"）
};

#endif // MEASUREMENTDIALOG_H
