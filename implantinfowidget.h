
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
#include "FramelessWindow.h"

class ImplantInfoWidget : public FramelessWindow
{
    Q_OBJECT

public:
    explicit ImplantInfoWidget(QWidget *parent = nullptr);
    ~ImplantInfoWidget();

signals:
    void openSettingsWindow();
    void implantReturnLogin();
    void onSignalFromImplantationSite();

private slots:
    void showImplantationSiteWidget(const QString &serial);
    bool insertNewSensor();

private:
    //标题
    QLabel *titleLabel;
    QLabel *serialLabel;
    QLabel *checksumLabel;
    QLabel *implantDoctorLabel;
    QLabel *treatDoctorLabel;
    QLabel *dateLabel;

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
//    CustomKeyboard *customkeyboard = nullptr;
    QLineEdit *currentEdit = nullptr;
    CustomKeyboard* currentKeyboard = nullptr;
    bool eventFilterInstalled = false;  // 用来跟踪事件过滤器的安装状态
    QString m_serial;
    QString m_locationChoice;   // 记录“左”或“右”
    bool insertImplantationSite();
    void changeEvent(QEvent *event) override;
};

#endif // IMPLANTINFOWIDGET_H
