#include "rhcinputdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>

RHCInputDialog::RHCInputDialog(QWidget *parent)
    : CloseOnlyWindow(parent)
{
    // 设置窗口标题
    setWindowTitle(tr("输入RHC"));
    this->setStyleSheet("background-color: #18273E;");
    this->setFixedSize(500,250);


    QVBoxLayout *mainlayout = new QVBoxLayout(this);
    mainlayout->setContentsMargins(15,15,15,15);

    // 创建网格布局
    QGridLayout *gridLayout = new QGridLayout();

    // 创建输入框和分隔符
    QLineEdit *raInput1 = new QLineEdit(this);

    QLineEdit *rvInput1 = new QLineEdit(this);
    QLineEdit *rvInput2 = new QLineEdit(this);
    QLabel *rvdivider = new QLabel("/", this);

    QLineEdit *paInput1 = new QLineEdit(this);
    QLineEdit *paInput2 = new QLineEdit(this);
    QLineEdit *paInput3 = new QLineEdit(this);
    QLabel *padivider = new QLabel("/", this);

    QLineEdit *pcwpInput1 = new QLineEdit(this);

    // 设置QLineEdit样式
    raInput1->setStyleSheet("QLineEdit { border: 1px solid #ccd1d9; border-radius: 5px; padding: 5px; background-color: #ffffff; color: black; }");
    rvInput1->setStyleSheet("QLineEdit { border: 1px solid #ccd1d9; border-radius: 5px; padding: 5px; background-color: #ffffff; color: black;}");
    rvInput2->setStyleSheet("QLineEdit { border: 1px solid #ccd1d9; border-radius: 5px; padding: 5px; background-color: #ffffff; color: black;}");
    paInput1->setStyleSheet("QLineEdit { border: 1px solid #ccd1d9; border-radius: 5px; padding: 5px; background-color: #ffffff; color: black;}");
    paInput2->setStyleSheet("QLineEdit { border: 1px solid #ccd1d9; border-radius: 5px; padding: 5px; background-color: #ffffff; color: black;}");
    paInput3->setStyleSheet("QLineEdit { border: 1px solid #ccd1d9; border-radius: 5px; padding: 5px; background-color: #ffffff; color: black;}");
    pcwpInput1->setStyleSheet("QLineEdit { border: 1px solid #ccd1d9; border-radius: 5px; padding: 5px; background-color: #ffffff; color: black;}");

    raInput1->setFocusPolicy(Qt::ClickFocus);
    rvInput1->setFocusPolicy(Qt::ClickFocus);
    rvInput2->setFocusPolicy(Qt::ClickFocus);
    paInput1->setFocusPolicy(Qt::ClickFocus);
    paInput2->setFocusPolicy(Qt::ClickFocus);
    paInput3->setFocusPolicy(Qt::ClickFocus);
    pcwpInput1->setFocusPolicy(Qt::ClickFocus);

    // 拿到单例键盘
    currentKeyboard = CustomKeyboard::instance(this);

    // 给每个 QLineEdit 注册一次偏移（如果你想要默认偏移都一样，就写同一个 QPoint）
    currentKeyboard->registerEdit(raInput1, QPoint(-170,0));
    currentKeyboard->registerEdit(rvInput1);
    currentKeyboard->registerEdit(rvInput2, QPoint(-170,0));
    currentKeyboard->registerEdit(paInput1);
    currentKeyboard->registerEdit(paInput2, QPoint(-170,0));
    currentKeyboard->registerEdit(paInput3 , QPoint(-170,0));
    currentKeyboard->registerEdit(pcwpInput1 , QPoint(-170,0));

    // 设置QLabel样式
    rvdivider->setStyleSheet("QLabel { font-size: 18px; color: #aaaaaa; }");
    padivider->setStyleSheet("QLabel { font-size: 18px; color: #aaaaaa; }");

    // 设置布局
    gridLayout->addWidget(new QLabel("RA", this), 0, 0);
    gridLayout->addWidget(raInput1, 0, 4);

    gridLayout->addWidget(new QLabel("RV", this), 1, 0);
    gridLayout->addWidget(rvInput1, 1, 1);
    gridLayout->addWidget(rvdivider, 1, 2);
    gridLayout->addWidget(rvInput2, 1, 3);

    gridLayout->addWidget(new QLabel("PA", this), 2, 0);
    gridLayout->addWidget(paInput1, 2, 1);
    gridLayout->addWidget(padivider, 2, 2);
    gridLayout->addWidget(paInput2, 2, 3);
    gridLayout->addWidget(paInput3, 2, 4);

    gridLayout->addWidget(new QLabel("PCWP", this), 3, 0);
    gridLayout->addWidget(pcwpInput1, 3, 4);


    // 添加按钮
    QHBoxLayout *btnlayout = new QHBoxLayout();
    clearButton = new QPushButton(tr("全部清除"), this);
    clearButton->setIcon(QIcon(":/image/delete.png"));
    clearButton->setFixedSize(120,35);
    saveButton = new QPushButton(tr("保存"), this);
    saveButton->setFixedSize(120,35);
    saveButton->setIcon(QIcon(":/image/icons8-save.png"));

    clearButton->setStyleSheet(R"(
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
        padding: 2px 5px;
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

    saveButton->setStyleSheet(R"(
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
        padding: 2px 5px;
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

    saveButton->setFixedWidth(120);
    btnlayout->addWidget(clearButton);
    btnlayout->addStretch();
    btnlayout->addWidget(saveButton);

    mainlayout->addLayout(gridLayout);
    mainlayout->addLayout(btnlayout);
}


RHCInputDialog::~RHCInputDialog() {
    // 释放资源
}

QString RHCInputDialog::getRHCValue() const {
    return rhcEdit->text();  // 返回在输入框中输入的值
}

void RHCInputDialog::changeEvent(QEvent *event){
    QWidget::changeEvent(event);
    if (event->type() == QEvent::LanguageChange) {
        // 系统自动发送的 LanguageChange
        setWindowTitle(tr("输入RHC"));
        clearButton->setText(tr("全部清除"));
        saveButton->setText(tr("保存"));
    }
}
