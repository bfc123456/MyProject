
#ifndef PATIENTSIGNALSTRENGTHWIDGET_H
#define PATIENTSIGNALSTRENGTHWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QPointer>
#include "Customkeyboard.h"
#include "FramelessWindow.h"
#include "MeasurementDialog.h"
#include "CircularProgressbar.h"
#include "SettingsWidget.h"

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
