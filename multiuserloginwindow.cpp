#include "multiuserloginwindow.h"
#include <QVBoxLayout>

MultiUserLoginWindow::MultiUserLoginWindow(QWidget *parent):FramelessWindow(parent)
{
    usernameEdit = new QLineEdit(this);
    passwordEdit =new QLineEdit(this);
    rememberMeCheck = new QCheckBox("记住我",this);
    loginButton = new QPushButton("登录",this);
    statusLabel = new QLabel(this);

    usernameEdit->setPlaceholderText("用户名");
    passwordEdit->setPlaceholderText("密码");
    passwordEdit->setEchoMode(QLineEdit::Password);

    QVBoxLayout *loginLayout = new QVBoxLayout(this);
    loginLayout->addWidget(usernameEdit);
    loginLayout->addWidget(passwordEdit);
    loginLayout->addWidget(rememberMeCheck);
    loginLayout->addWidget(loginButton);
    loginLayout->addWidget(statusLabel);

    connect(loginButton,&QPushButton::clicked,this,&MultiUserLoginWindow::onLoginClicked);

    setLayout(loginLayout);
    setWindowTitle("多用户登录");
    resize(300,200);

}

void MultiUserLoginWindow::onLoginClicked(){
    statusLabel->setText("正在验证，请稍后...");
}
