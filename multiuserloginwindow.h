#ifndef MULTIUSERLOGINWINDOW_H
#define MULTIUSERLOGINWINDOW_H

#include "FramelessWindow.h"
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>

class MultiUserLoginWindow : public FramelessWindow{
    Q_OBJECT

public:
    explicit MultiUserLoginWindow(QWidget *parent = nullptr);    //防止隐式转换

signals:
    void loginSuccess(const QString &username);

private slots:
    void onLoginClicked();

private:
    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;
    QCheckBox *rememberMeCheck;
    QPushButton *loginButton;
    QLabel *statusLabel;

//    void setupUI();
    void simulateLogin();   //暂时模拟登录
};

#endif // MULTIUSERLOGINWINDOW_H
