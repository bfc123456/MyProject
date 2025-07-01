
#ifndef FOLLOWUPFORM_H
#define FOLLOWUPFORM_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include "customkeyboard.h"
#include "FramelessWindow.h"
#include "measurementdialog.h"
#include "circularprogressbar.h"
#include "settingswidget.h"

class FollowUpForm: public FramelessWindow
{
    Q_OBJECT

public:
    explicit FollowUpForm (QWidget *parent = nullptr);
    ~FollowUpForm ();

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
    MeasurementDialog *measurementDialog;
    SettingsWidget *settingswidgetMrasure;

    CustomKeyboard *currentKeyboard = nullptr;
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

#endif // FOLLOWUPFORM_H
