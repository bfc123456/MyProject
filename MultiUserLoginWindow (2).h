
#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QElapsedTimer>
#include <QTimer>
#include <QDebug>
#include <QTranslator>
#include <QElapsedTimer>
#include "maintenancewidget.h"
#include "ImplantRegistrationWidget.h"
#include "CloseOnlyWindow.h"
#include "FramelessWindow.h"
#include "udpdebugwidget.h"
#include "customcombobox.h"
#include <memory>
#include <QStackedWidget>
#include "customkeyboard.h"
#include "settingswidget.h"

class FollowUpForm;   // 前向声明

class LoginWindow : public FramelessWindow   {
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow();
    void changeLanguage(const QString &languageCode);
    void openSettingsWindow();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QVBoxLayout *mainLayout;
    QLabel *titleLabel;
    CustomComboBox *usernameCombox;
    QLineEdit *passwordEdit;
    QPushButton *loginButton;
    QLabel *errorLabel;
    QElapsedTimer clickTimer; //连续点击记录器
    QTranslator translator;//翻译器
    CustomKeyboard* currentKeyboard;//虚拟键盘

    QStackedWidget *stackedWidget;
    SettingsWidget *settingswidget;
    std::unique_ptr<ImplantInfoWidget> implantWindow;
    std::unique_ptr<FollowUpForm> followupformwindow;
    std::unique_ptr<MaintenanceWidget> maintenancewidget;
    std::unique_ptr<udpDebugWidget> udpdebugwidget;

    bool isInitialized = false; //初始化标志位
    //错误标签
    enum ErrorType { NoError = 0, ErrLength, ErrAuth };
    ErrorType   m_lastError = NoError;

    void showHiddenWidget();
    void changeEvent(QEvent *event) override;

private slots:
    void onSettingClicked();
    void openImplantWindow();
    void closeImplantWindow();
    void showLoginWindow();
    void openFollowupFormWindow();
    void closeFollowupformwindow();
    void onLoginClicked();
    void clearErrorMessage();  // 用于清除错误信息
};


#endif // LOGINWINDOW_H
