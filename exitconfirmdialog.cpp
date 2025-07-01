#include "exitconfirmdialog.h"
#include <QIcon>
#include <QStyle>
#include <QGraphicsDropShadowEffect>
#include <QGuiApplication>
#include <QScreen>

ExitConfirmDialog::ExitConfirmDialog(QWidget *parent)
    : CloseOnlyWindow(parent)
{
    // 设置无边框 + 遮罩背景
    // 获取屏幕分辨率
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int screenWidth = screenGeometry.width();
    int screenHeight = screenGeometry.height();

    // 计算缩放比例
    scaleX = (float)screenWidth / 1024;
    scaleY = (float)screenHeight / 600;

    resize(400*scaleX, 200*scaleY);

    QWidget *mainWidget = new QWidget(this);
    mainWidget->setObjectName("mainWidget");
    mainWidget->setStyleSheet(R"(
        #mainWidget {
            background-color: rgba(30, 30, 30, 230);
            border-radius: 10px;
        }
    )");

    QVBoxLayout *mainLayout = new QVBoxLayout(mainWidget);
    mainLayout->setContentsMargins(20*scaleX, 20*scaleY, 20*scaleX, 20*scaleY);
    mainLayout->setSpacing(20*scaleY);

    // 顶部关闭按钮
    closeButton = new QPushButton("✖");
    closeButton->setFixedSize(24*scaleX, 24*scaleY);
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

    shutdownButton->setFixedSize(100*scaleX, 32*scaleY);
    returnButton->setFixedSize(100*scaleX, 32*scaleY);

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
    buttonLayout->addSpacing(40*scaleX);
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
