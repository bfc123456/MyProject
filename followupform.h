#ifndef FOLLOWUPFORM_H
#define FOLLOWUPFORM_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QDateEdit>
#include <QPushButton>
#include "customkeyboard.h"


class FollowUpForm: public QWidget
{
    Q_OBJECT

public:
    explicit FollowUpForm (QWidget *parent = nullptr);
    ~FollowUpForm ();

protected:
    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent *event) override;

signals:
    void openSettingsWindow();
    void followReturnToLogin();
    void onSignalFromImplantationSite();

private:
    // 输入字段
    QLineEdit *serialInput;
    QLineEdit *checksumInput;
    QLineEdit *implantDoctorInput;
    QLineEdit *treatDoctorInput;
    QDateEdit *implantDateInput;

    // 操作按钮
    QPushButton *backButton;
    QPushButton *continueButton;

    CustomKeyboard* currentKeyboard = nullptr;
    bool eventFilterInstalled = false;  // 用来跟踪事件过滤器的安装状态

private slots:
    void showImplantMonitorWidget();
};

#endif // FOLLOWUPFORM_H
