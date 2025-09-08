#ifndef IMPLANTMONITOR_H
#define IMPLANTMONITOR_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include "ModernWaveplot.h"
#include "ReadoutrecordDialog.h"
#include "ReviewWidget.h"
#include "FramelessWindow.h"
#include "CircularProgressbar.h"
#include "RhcinputDialog.h"
#include "DatabaseManager.h"
#include "CustomMessagebox.h"
#include "Global.h"

/**
 * @class ImplantMonitor
 * @brief 植入监测界面（医生端）
 *
 * 本界面用于医生在植入场景下对传感器进行实时监测和数据采集。
 * 主要功能包括：
 * - 显示植入物的基本信息（传感器 ID、植入位置等）
 * - 实时显示血压、平均压、心率等关键生理参数
 * - 提供波形绘制和信号强度进度条，便于医生观察数据质量
 * - 可进入心输出量（CO）和 RHC 输入界面，补充临床信息
 * - 提供审计、回顾和读数记录功能，支持医生查看历史测量结果
 * - 提供导出功能，便于保存和分析测量数据
 *
 * @note 本类通常由植入注册（ImplantRegistrationWidget）完成后进入，
 *       是医生执行植入测量和随访的主要操作界面。
 */


class ImplantMonitor : public FramelessWindow {
    Q_OBJECT
public:
    explicit ImplantMonitor(QWidget *parent = nullptr, const QString &sensorId = QString());
    ~ImplantMonitor();

private:
    // UI 控件
    QLabel *titleLabel;
    QLabel *idLabel;
    QLabel *bpVal;        // 血压显示
    QLabel *avgVal;       // 平均压显示
    QLabel *hrVal;        // 心率显示
    QPushButton *startBtn;// 开始测量按钮
    QPushButton *inputCO; // 心输出量按钮
    QPushButton *inputRHC;// RHC 按钮
    QPushButton *statBtn; // 审计界面按钮
    QPushButton *statisticsbtn;// 读数记录按钮
    CircularProgressBar *progressBar;// 信号进度
    ModernWavePlot *plot;            // 波形图

    // 数据与状态
    ReadoutRecordDialog *readoutdialog = nullptr; // 读数记录对话框
    ReviewWidget *reviewwidget = nullptr;         // 回顾界面
    QString m_serial;                             // 传感器ID
    bool m_isLeft = true;                         // 植入位置（左/右）
    bool isMeasuring = false;                     // 测量状态标记
    float scaleX = 1.0f, scaleY = 1.0f;           // 屏幕缩放比例
    QVector<QPointF> m_plotPoints;                // 波形数据点
    QList<MeasurementData> measurementList;       // 测量记录

    // 核心方法
    void initWorkersAndConnections(); // 初始化线程与连接

signals:
    void dataListUpdated(const QList<MeasurementData>& list); // 通知表格更新
    void returnImplantationsite();                            // 返回植入位置界面

private slots:
    void startMeasurement();  // 启动测量（主动控制）
    void stopMeasurement();   // 停止测量（主动控制）
    void updateWaveform(const MeasurementData &data); // 波形更新
    void openCOClicked();     // 心输出量对话框
    void openRHCClicked();    // RHC 对话框
    void openReviewClicked(); // 回顾界面
    void onReadoutButtonClicked(); // 读数记录
    void onRowDeleted(int row);    // 删除记录
    void OpenSettingsRequested();  // 设置界面
    void openSaveConfirm();        // 保存确认
    void changeEvent(QEvent *event) override; // 语言切换
    void exportCurrentData(); // 点击按钮触发的输出函数 //测试使用
};

#endif // IMPLANTMONITOR_H
