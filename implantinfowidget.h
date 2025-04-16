#ifndef IMPLANTINFOWIDGET_H
#define IMPLANTINFOWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QDateEdit>
#include <QPushButton>
#include <QCloseEvent>
#include "customkeyboard.h"
#include "customdateedit.h"
#include "implantationsite.h"

class ImplantInfoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ImplantInfoWidget(QWidget *parent = nullptr);
    ~ImplantInfoWidget();

signals:
    void openSettingsWindow();
    void implantReturnLogin();
    void onSignalFromImplantationSite();

protected:
    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent *event) override;

private slots:
    void showImplantationSiteWidget();

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
    ImplantationSite *implantationSite;
    CustomKeyboard *customkeyboard = nullptr;
    QLineEdit *currentEdit = nullptr;
    CustomKeyboard* currentKeyboard = nullptr;
    bool eventFilterInstalled = false;  // 用来跟踪事件过滤器的安装状态

};

#endif // IMPLANTINFOWIDGET_H
