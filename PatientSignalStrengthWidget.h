
#ifndef PATIENTSIGNALSTRENGTHWIDGET_H
#define PATIENTSIGNALSTRENGTHWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QPointer>
#include "customkeyboard.h"
#include "framelesswindow.h"
#include "measurementdialog.h"
#include "circularprogressbar.h"
#include "settingswidget.h"

/**
 * @class PatientSignalStrengthWidget
 * @brief 术区/贴片信号强度检测与引导窗口（测量前置环节）
 *
 * 本窗口用于在开始正式测量之前，实时检测并展示患者（或植入/贴附点）的
 * 信号强度，满足阈值后方可进入测量流程。提供返回登录、打开设置、开始
 * 测量等入口，并支持叠层（overlay）与无边框样式的交互体验。
 *
 * 主要职责：
 * - 实时刷新信号强度并以环形进度条展示（CircularProgressBar）
 * - 达到门限时发出 progressThresholdReached()，允许进入测量对话框
 * - 打开测量设置（SettingsWidget）或测量对话框（MeasurementDialog）
 * - 多语言切换时动态更新文案
 *
 * 信号：
 * - openSettingsWindow()：请求打开设置面板
 * - followReturnToLogin()：引导返回登录/上一级
 * - onSignalFromImplantationSite()：检测到来自植入/贴附位置信号
 * - progressThresholdReached()：信号强度达到门限（可开始测量）
 *
 * 使用示例：
 * @code
 * auto *w = new PatientSignalStrengthWidget(parent);
 * connect(w, &PatientSignalStrengthWidget::progressThresholdReached,
 *         w, &PatientSignalStrengthWidget::openMeasurementDialog);
 * w->show();
 * @endcode
 */

class PatientSignalStrengthWidget: public FramelessWindow
{
    Q_OBJECT

public:
    explicit PatientSignalStrengthWidget (QWidget *parent = nullptr);
    ~PatientSignalStrengthWidget ();

signals:
    void openSettingsWindow();
    void followReturnToLogin();
    void onSignalFromImplantationSite();
    void progressThresholdReached();

private:
    //标题
    QLabel *operationTips;

    // 操作按钮
    QPushButton *backButton;
    QLabel *labelSensor;
    QString m_serial;
    CircularProgressBar *progress;
    int continuousTime = 0;
    QTimer *timer;
//    MeasurementDialog *measurementDialog = nullptr; // 对话框;
    SettingsWidget *settingswidgetMrasure = nullptr; // 对话框;
    CustomKeyboard *currentKeyboard = nullptr;
//    QWidget* overlay = nullptr;
    QPointer<MeasurementDialog> dlg;
    QPointer<QWidget> overlay;
    // 遮罩
    bool eventFilterInstalled = false;  // 用来跟踪事件过滤器的安装状态
    void validateForm();
    void changeEvent(QEvent *event) override;
    QString fetchSensorIds() const; //保障不修改对象状态

private slots:
//    void showImplantionSite();
    void checkProgress();
    void openMeasurementDialog();
    void openMeasureSettingsWindow();
};

#endif // PatientSignalStrengthWidget_H
