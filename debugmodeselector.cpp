#include "debugmodeselector.h"
#include <QBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QGuiApplication>
#include <QScreen>

DebugModeSelector::DebugModeSelector(QWidget *parent) : CloseOnlyWindow(parent)
{
    // 获取屏幕分辨率
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int screenWidth = screenGeometry.width();
    int screenHeight = screenGeometry.height();

    // 计算缩放比例
    scaleX = (float)screenWidth / 1024;
    scaleY = (float)screenHeight / 600;

    setFixedSize(400*scaleX, 280*scaleY);

    setStyleSheet(R"(
        QDialog {
            background-color: #333333; /* 深灰色背景 */
            border: 2px solid white;   /* 白色外边框 */
            border-radius: 10px;
            color: white;
            font-size: 14px;
        }
    )");

    // 创建关闭按钮
    QPushButton *closeButton = new QPushButton(this);
    closeButton->setIcon(QIcon(":/image/icons-close.png"));
    closeButton->setIconSize(QSize(20 * scaleX, 20 * scaleX));
    closeButton->setStyleSheet(R"(
        QPushButton {
            background-color: transparent;
            color: white;
            font-size: 18px;
            font-weight: bold;
            border: none;
            padding: 5px 10px;
        }
        QPushButton:hover {
            color: #CCCCCC; /*  hover 时浅灰 */
        }
    )");
    closeButton->setFixedSize(30, 30);  // 设置按钮的大小

    // 创建按钮布局，用于设置关闭按钮的显示位置
    QHBoxLayout *headerLayout = new QHBoxLayout();
    headerLayout->setContentsMargins(0, 0, 0, 0);  // 移除间距
    headerLayout->addWidget(closeButton, 0, Qt::AlignRight);  // 将关闭按钮放置在右侧

    QVBoxLayout *mainlayout = new QVBoxLayout(this);
    QLabel *tittle = new QLabel(tr("请选择调试类型"),this);
    tittle->setAlignment(Qt::AlignCenter);
    tittle->setStyleSheet(R"(
        font-size: 20px;
        font-weight: bold;
        background-color: transparent;
    )");

    mainlayout->addLayout(headerLayout);
    mainlayout->addStretch();
    mainlayout->addWidget(tittle);
    QHBoxLayout *tittlelayout = new QHBoxLayout();
    QPushButton *serialBtn = new QPushButton(tr("串口\n调试"),this);
    QPushButton *udpBtn = new QPushButton(tr("网络\n调试"),this);
    serialBtn->setMinimumSize(100*scaleX, 100*scaleY);
    udpBtn->setMinimumSize(100*scaleX, 100*scaleY);

    serialBtn->setStyleSheet(R"(
                   QPushButton {
                       background-color: #2196F3; /* 蓝色按钮 */
                       border: none;
                       border-radius: 6px;
                       color: white;
                       font-weight: bold;
                       font-size: 14px;
                       padding: 8px 20px;
                   }
                   QPushButton:pressed {
                       background-color: #1976D2; /* 按下时更深的蓝色 */
                       padding-left: 2px;
                       padding-top: 2px;
                   }
               )");


    udpBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #2196F3; /* 蓝色按钮 */
            border: none;
            border-radius: 6px;
            color: white;
            font-weight: bold;
            font-size: 14px;
            padding: 8px 20px;
        }
        QPushButton:pressed {
            background-color: #1976D2; /* 按下时更深的蓝色 */
            padding-left: 2px;
            padding-top: 2px;
        }
    )");

    mainlayout->addSpacing(50*scaleY);
    tittlelayout->addWidget(serialBtn);
    tittlelayout->addSpacing(100*scaleY);
    tittlelayout->addWidget(udpBtn);
    tittlelayout->setContentsMargins(50*scaleX,0,50*scaleX,0);
    mainlayout->addLayout(tittlelayout);
    mainlayout->addStretch();

    connect(serialBtn,&QPushButton::clicked,this,[=](){
        emit modeSelected("serial");
        this->accept();
    });

    connect(udpBtn,&QPushButton::clicked,this,[=](){
       emit modeSelected("udp");
        this->accept();
    });

    // 连接关闭按钮的信号
    connect(closeButton, &QPushButton::clicked, this, &DebugModeSelector::close);

}
