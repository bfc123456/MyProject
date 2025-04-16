#include "exitconfirmdialog.h"
#include <QIcon>
#include <QStyle>
#include <QGraphicsDropShadowEffect>

ExitConfirmDialog::ExitConfirmDialog(QWidget *parent)
    : QDialog(parent)
{
    // 设置无边框 + 遮罩背景
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground);
    resize(400, 200);

    QWidget *mainWidget = new QWidget(this);
    mainWidget->setObjectName("mainWidget");
    mainWidget->setStyleSheet(R"(
        #mainWidget {
            background-color: rgba(30, 30, 30, 230);
            border-radius: 10px;
        }
    )");

    QVBoxLayout *mainLayout = new QVBoxLayout(mainWidget);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(20);

    // 顶部关闭按钮
    closeButton = new QPushButton("✖");
    closeButton->setFixedSize(24, 24);
    closeButton->setStyleSheet("QPushButton { color: white; background: transparent; border: none; font-size: 14px; }");
    connect(closeButton, &QPushButton::clicked, this, &QDialog::reject);

    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->addStretch();
    topLayout->addWidget(closeButton);
    mainLayout->addLayout(topLayout);

    // 提示文本
    messageLabel = new QLabel("确定退出此界面");
    messageLabel->setAlignment(Qt::AlignCenter);
    messageLabel->setStyleSheet("color: white; font-size: 16px;");
    mainLayout->addWidget(messageLabel);

    // 按钮布局
    shutdownButton = new QPushButton(" 关闭系统");
    shutdownButton->setIcon(QIcon(":/icons/shutdown.png"));  // 替换为你的图标路径
    returnButton = new QPushButton(" 返回主界面");
    returnButton->setIcon(QIcon(":/icons/home.png"));

    shutdownButton->setFixedSize(100, 32);
    returnButton->setFixedSize(100, 32);

    QString buttonStyle = R"(
        QPushButton {
            background-color: #2e8bff;
            color: white;
            border: none;
            border-radius: 4px;
            font-size: 13px;
            padding: 6px;
        }
        QPushButton:hover {
            background-color: #5599ff;
        }
    )";

    shutdownButton->setStyleSheet(buttonStyle);
    returnButton->setStyleSheet(buttonStyle);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(shutdownButton);
    buttonLayout->addSpacing(40);
    buttonLayout->addWidget(returnButton);
    buttonLayout->addStretch();

    mainLayout->addLayout(buttonLayout);

    QVBoxLayout *outerLayout = new QVBoxLayout(this);
    outerLayout->addWidget(mainWidget);

    // 信号连接
    connect(shutdownButton, &QPushButton::clicked, this, &ExitConfirmDialog::shutdownClicked);
    connect(returnButton, &QPushButton::clicked, this, &ExitConfirmDialog::returnHomeClicked);
}

ExitConfirmDialog::~ExitConfirmDialog(){

}
