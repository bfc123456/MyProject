#include "debugmodeselector.h"
#include <QBoxLayout>
#include <QPushButton>
#include <QLabel>

DebugModeSelector::DebugModeSelector(QWidget *parent) : CloseOnlyWindow(parent)
{
     setFixedSize(400, 280);

    this->setStyleSheet(R"(
        QWidget {
             background-color: qlineargradient(
                 x1: 0, y1: 0, x2: 0, y2: 1,
                 stop: 0 rgba(30, 50, 80, 0.9),     /* 顶部：偏亮蓝灰，透明度 0.9 */
                 stop: 1 rgba(10, 25, 50, 0.75)     /* 底部：深蓝，透明度 0.75 */
             );
            color: white;
            font-size: 14px;
            border-radius: 10px;
        }
    )");

    QVBoxLayout *mainlayout = new QVBoxLayout(this);
    QLabel *tittle = new QLabel(tr("请选择调试类型"),this);
    tittle->setAlignment(Qt::AlignCenter);
    tittle->setStyleSheet(R"(
        font-size: 20px;
        font-weight: bold;
        background-color: transparent;
    )");

    mainlayout->addStretch();
    mainlayout->addWidget(tittle);
    QHBoxLayout *tittlelayout = new QHBoxLayout(this);
    QPushButton *serialBtn = new QPushButton("串口\n调试",this);
    QPushButton *udpBtn = new QPushButton("网络\n调试",this);
    serialBtn->setMinimumSize(100, 100);
    udpBtn->setMinimumSize(100, 100);

    serialBtn->setStyleSheet(R"(
        QPushButton {
            background-color: qlineargradient(
                stop: 0 rgba(110, 220, 145, 180),
                stop: 1 rgba(58, 170, 94, 180)
            );
            border: 1px solid rgba(168, 234, 195, 0.6);
            border-radius: 6px;
            color: white;
            font-weight: bold;
            font-size: 18px;
            padding: 10px 20px;
        }

        QPushButton:pressed {
            background-color: qlineargradient(
                stop: 0 rgba(44, 128, 73, 200),
                stop: 1 rgba(29, 102, 53, 200)
            );
            padding-left: 2px;
            padding-top: 2px;
        }
    )");


    udpBtn->setStyleSheet(R"(
        QPushButton {
            background-color: qlineargradient(
                stop: 0 rgba(135, 165, 250, 180),
                stop: 1 rgba(100, 120, 220, 180)
            );
            border: 1px solid rgba(190, 200, 250, 0.5);
            border-radius: 6px;
            color: white;
            font-weight: bold;
            font-size: 18px;
            padding: 10px 20px;
        }

        QPushButton:pressed {
            background-color: qlineargradient(
                stop: 0 rgba(70, 90, 180, 200),
                stop: 1 rgba(50, 70, 160, 200)
            );
            padding-left: 2px;
            padding-top: 2px;
        }
    )");

    mainlayout->addSpacing(50);
    tittlelayout->addWidget(serialBtn);
    tittlelayout->addSpacing(100);
    tittlelayout->addWidget(udpBtn);
    tittlelayout->setContentsMargins(50,0,50,0);
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

}
