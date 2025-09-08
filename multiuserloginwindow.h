
#ifndef MULTIUSERLoginWindow_H
#define MULTIUSERLoginWindow_H

#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QDebug>
#include <memory>
#include <QPushButton>
#include <QVBoxLayout>
#include <QTranslator>
#include <QElapsedTimer>
#include <QElapsedTimer>
#include <QStackedWidget>
#include "CloseOnlyWindow.h"
#include "FramelessWindow.h"
#include "UdpDebugWidget.h"
#include "CustomCombobox.h"
#include "Customkeyboard.h"
#include "SettingsWidget.h"
#include "SerialDebugWidget.h"
#include "ImplantRegistrationWidget.h"

class PatientSignalStrengthWidget;   // 前向声明

/**
 * @class MultiUserLoginWindow
 * @brief 多用户登录窗口界面
 *
 * 该界面主要用于用户在启动时选择系统模式（如“家用模式”或“植入模式”），并进行简单的身份验证。
 * 用户通过下拉框选择角色并输入密码，若验证通过，将进入对应的功能界面：
 * - 家用模式 -> 打开 PatientSignalStrengthWidget 用户测量界面
 * - 植入模式 -> 打开 ImplantRegistrationWidget 植入注册界面
 *
 * 除了基本的登录功能，本界面还包含以下特性：
 * - 提供设置按钮，允许用户进入 SettingsWidget 修改系统参数
 * - 支持多语言切换，通过 QTranslator 动态加载翻译文件
 * - 支持虚拟键盘输入，方便触屏设备操作
 * - 连续快速点击设置按钮五次可进入维护界面（MaintenanceWidget），
 *   在维护界面中可进一步进入串口调试（MaintenanceWidget）或 UDP 调试（udpDebugWidget）
 *
 * @note 该窗口继承自 FramelessWindow，采用无边框风格，并支持自定义绘制。
 */

class MultiUserLoginWindow : public FramelessWindow   {
    Q_OBJECT

public:
    explicit MultiUserLoginWindow(QWidget *parent = nullptr);
    ~MultiUserLoginWindow();
    void changeLanguage(const QString &languageCode);    // 切换界面语言
    void openSettingsWindow();    // 打开设置界面

protected:
    void paintEvent(QPaintEvent *event) override;    // 窗口自绘（无边框风格）

private:

    QVBoxLayout *mainLayout;

    QLabel *titleLabel;
    CustomComboBox *usernameCombox;    // 用户角色选择下拉框（家用模式/植入模式）
    QLineEdit *passwordEdit;
    QPushButton *loginButton;
    QLabel *errorLabel;
    QElapsedTimer clickTimer;    //连续点击记录器
    QTranslator translator;    // 界面翻译器（中英文切换）
    CustomKeyboard* currentKeyboard;    //虚拟键盘

    QStackedWidget *stackedWidget;
    SettingsWidget *settingswidget;
    std::unique_ptr<ImplantRegistrationWidget> implantRegistrationWindow;
    std::unique_ptr<PatientSignalStrengthWidget> PatientSignalStrengthWidgetwindow;
    std::unique_ptr<SerialDebugWidget> serialDebugWidget;
    std::unique_ptr<udpDebugWidget> udpdebugwidget;

    bool isInitialized = false; //初始化标志位
    enum ErrorType { NoError = 0, ErrLength, ErrAuth };    //错误标签
    ErrorType   m_lastError = NoError;

    void showHiddenWidget();
    void changeEvent(QEvent *event) override;

private slots:
    void onSettingClicked();
    void openImplantRegistrationWidget();    // 打开植入注册界面
    void closeImplantRegistrationWidget();
    void showMultiUserLoginWindow();
    void openPatientSignalStrengthWidgetWindow();    // 打开用户测量界面
    void closePatientSignalStrengthWidgetwindow();
    void onLoginClicked();    // 登录按钮点击事件
    void clearErrorMessage();    // 用于清除错误信息
};


#endif // MultiUserLoginWindow_H
