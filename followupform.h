#ifndef FOLLOWUPFORM_H
#define FOLLOWUPFORM_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QDateEdit>
#include <QPushButton>
#include <QComboBox>
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
    QComboBox *selectcomboBox;

    // 操作按钮
    QPushButton *backButton;
    QPushButton *continueButton;

    CustomKeyboard* currentKeyboard = nullptr;
    bool eventFilterInstalled = false;  // 用来跟踪事件过滤器的安装状态
    void validateForm();


private slots:
    void showImplantionSite();
};

#endif // FOLLOWUPFORM_H
