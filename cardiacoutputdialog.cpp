#include "cardiacoutputdialog.h"
#include "customkeyboard.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>

CardiacOutputDialog::CardiacOutputDialog(QString bpValue, QString avgValue, QWidget *parent)
    : QDialog(parent)
{
    // 设置窗体样式
    setWindowTitle(tr("输入心输出量"));
    this->setStyleSheet("background-color: #18273E;");

    // 主布局
    QVBoxLayout *mainlayout = new QVBoxLayout(this);
    mainlayout->setContentsMargins(15,15,15,15);

    // 血压/平均值布局
    QLabel *bpLabel = new QLabel(bpValue,this);
    QLabel *avgLabel = new QLabel(avgValue,this);
    QLabel *unitLabel = new QLabel("mmHg",this);

    // 创建数据显示布局
    QVBoxLayout *dataunitLayout = new QVBoxLayout();
    QHBoxLayout *dataLayout = new QHBoxLayout();
    dataLayout->addWidget(bpLabel);
    dataLayout->addSpacing(10);
    dataLayout->addWidget(avgLabel);
    dataunitLayout->addLayout(dataLayout);
    dataunitLayout->addWidget(unitLabel);
    mainlayout->addLayout(dataunitLayout);

    bpLabel->setStyleSheet("background-color: transparent; color: white; font-size: 16px;");
    avgLabel->setStyleSheet("background-color: transparent; color: white; font-size: 16px;");
    unitLabel->setStyleSheet("background-color: transparent; color: white; font-size: 16px;");

    // 心输出量输入框
    QLabel *coTitle = new QLabel("心输出量",this);
    coTitle->setStyleSheet("background-color: transparent; color: white; font-size: 16px;");
    coTitle->setFixedHeight(40);
    coEdit = new QLineEdit;
    coEdit->setFixedSize(135,35);
    coEdit->setStyleSheet(R"(
        QLineEdit {
            background-color: #white;
            border: 1px solid rgba(255,255,255,0.15);
            border-radius: 8px;
            padding: 6px 12px;
            font-size: 16px;
            color: white;
        }
        QLineEdit:focus {
            border: 1px solid #4FA3F7;
            background-color: #253646;
        }
    )");

    coEdit->setFocusPolicy(Qt::ClickFocus);
//    coEdit->installEventFilter(CustomKeyboard::instance(this));
//    CustomKeyboard::instance()->registerEdit(coEdit, QPoint(-180, 0));

    QLabel *coUnit = new QLabel("L/Min",this);
    coUnit->setStyleSheet("background-color: transparent; color: white; font-size: 16px;");
    coUnit->setFixedHeight(40);

    // 创建输入框布局
    QHBoxLayout *coLayout = new QHBoxLayout();
    coLayout->addStretch();
    coLayout->addWidget(coTitle);
    coLayout->addSpacing(10);
    coLayout->addWidget(coEdit);
    coLayout->addSpacing(5);
    coLayout->addWidget(coUnit);
    coLayout->addStretch();
    mainlayout->addLayout(coLayout);

    // 设置按钮样式
    QPushButton *backBtn = new QPushButton(tr("返回"),this);
    QPushButton *okBtn = new QPushButton(tr("保存"),this);
    backBtn->setFixedSize(120,45);
    okBtn->setFixedSize(120,45);

    backBtn->setStyleSheet(R"(
        QPushButton {
            background-color: qlineargradient(
                x1:0, y1:0, x2:0, y2:1,
                stop:0 rgba(95, 169, 246, 180),
                stop:1 rgba(49, 122, 198, 180)
            );
            border: 1px solid rgba(163, 211, 255, 0.6);
            border-radius: 6px;
            color: white;
            font-weight: bold;
            font-size: 14px;
            padding: 8px 20px;
        }
        QPushButton:pressed {
            background-color: qlineargradient(
                stop: 0 rgba(47, 106, 158, 200),
                stop: 1 rgba(31, 78, 121, 200)
            );
            padding-left: 2px;
            padding-top: 2px;
        }
    )");

    okBtn->setStyleSheet(R"(
        QPushButton {
            background-color: qlineargradient(
                stop: 0 rgba(110, 220, 145, 180),
                stop: 1 rgba(58, 170, 94, 180)
            );
            border: 1px solid rgba(168, 234, 195, 0.6);
            border-radius: 6px;
            color: white;
            font-weight: bold;
            font-size: 14px;
            padding: 8px 20px;
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

    // 返回按钮与保存按钮的布局
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addWidget(backBtn);
    btnLayout->addStretch(1);
    btnLayout->addWidget(okBtn);
    mainlayout->addLayout(btnLayout);

    // 连接按钮的信号槽
    connect(backBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(okBtn, &QPushButton::clicked, this, &QDialog::accept);
}

CardiacOutputDialog::~CardiacOutputDialog() {
}

QString CardiacOutputDialog::getCOValue() const {
    return coEdit->text();
}
