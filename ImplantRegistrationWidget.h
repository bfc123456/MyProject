
#ifndef IMPLANTREGISTRATIONWIDGET_H
#define IMPLANTREGISTRATIONWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QDateEdit>
#include <QPushButton>
#include <QCloseEvent>
#include "Customkeyboard.h"
#include "CustomDateedit.h"
#include "ImplantAtionSite.h"
#include "FramelessWindow.h"

/**
 * @class ImplantRegistrationWidget
 * @brief 植入注册界面（医生端）
 *
 * 本界面主要用于医生在植入流程中填写并绑定传感器与患者信息，
 * 以确保后续测量数据能够正确关联到对应患者。
 *
 * 功能包括：
 * - 输入并校验传感器序列号、患者基本信息
 * - 将绑定信息写入数据库，保证可追溯性
 * - 提供返回与确认操作，防止误操作
 * - 作为登录后的“植入模式”主入口界面
 *
 * @note 本类通常由 MultiUserLoginWindow 在选择“植入模式”后打开。
 *       界面逻辑偏向医疗人员使用，需符合审计日志与数据追溯规范。
 */


class ImplantRegistrationWidget : public FramelessWindow
{
    Q_OBJECT

public:
    explicit ImplantRegistrationWidget(QWidget *parent = nullptr);
    ~ImplantRegistrationWidget();

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
    CustomMessageBox dlg;
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

    float scaleX;
    float scaleY;
};

#endif // ImplantRegistrationWidget_H
