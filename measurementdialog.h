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
#include "MeasurementTrendWidget.h"
#include "MeasurementData.h"  // 包含独立的结构体定义
#include "Global.h"

//前向声明
class DeviceAcquisitionWorker;
class MeasurementDataProcessor;

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
