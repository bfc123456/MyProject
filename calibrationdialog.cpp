#include "calibrationdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFont>
#include <QMessageBox>
#include "customkeyboard.h"
#include <QGuiApplication>
#include <QScreen>

CalibrationDialog::CalibrationDialog(QWidget* parent)
    : CloseOnlyWindow(parent)
{
    // 获取屏幕分辨率
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int screenWidth = screenGeometry.width();
    int screenHeight = screenGeometry.height();

    // 计算缩放比例
    scaleX = (float)screenWidth / 1024;
    scaleY = (float)screenHeight / 600;

    setWindowTitle(tr("校准传感器"));
    setFixedSize(400*scaleX, 280*scaleY);
    setStyleSheet("font-family: 'Microsoft YaHei'; font-size: 14px;");

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    title = new QLabel(tr("校准传感器"));
    title->setFixedWidth(180*scaleX);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-weight: bold; font-size: 16px; background-color: transparent; color: white;");
    mainLayout->addWidget(title, 0, Qt::AlignHCenter);

    // 数值显示
    QHBoxLayout* valueLayout = new QHBoxLayout;
    QLabel* bpValue = new QLabel("120/80");
    bpValue->setFixedWidth(80*scaleX);
    bpValue->setAlignment(Qt::AlignCenter);
    bpValue->setStyleSheet("font-weight: bold; font-size: 16px; background-color: transparent; color: white;");
    QLabel* pulseValue = new QLabel("94");
    pulseValue->setFixedWidth(80*scaleX);
    pulseValue->setAlignment(Qt::AlignCenter);
    pulseValue->setStyleSheet("font-weight: bold; font-size: 16px; background-color: transparent; color: white;");
    QLabel* unit = new QLabel("mmHg");
    unit->setFixedWidth(120*scaleX);
    unit->setAlignment(Qt::AlignCenter);
    unit->setStyleSheet("font-weight: bold; font-size: 16px; background-color: transparent; color: white;");

    QFont valueFont;
    valueFont.setPointSize(22);
    valueFont.setBold(true);
    bpValue->setFont(valueFont);
    pulseValue->setFont(valueFont);

    valueLayout->addStretch();
    valueLayout->addWidget(bpValue);
    valueLayout->addSpacing(10*scaleX);
    valueLayout->addWidget(pulseValue);
    valueLayout->addStretch();

    mainLayout->addLayout(valueLayout);
    mainLayout->addWidget(unit, 0, Qt::AlignHCenter);

    // 输入区域
    QHBoxLayout* inputLayout = new QHBoxLayout;
    inputLabel = new QLabel(tr("PA导管平均值"));
    inputLabel->setFixedWidth(150*scaleX);
    inputLabel->setFixedHeight(40*scaleY);
    inputLabel->setAlignment(Qt::AlignCenter);
    inputLabel->setStyleSheet("font-weight: bold; font-size: 16px; background-color: transparent; color: white;");
    inputEdit = new QLineEdit();
    inputEdit->setPlaceholderText(tr("输入校准值..."));
    inputEdit->setFixedSize(120*scaleX,35*scaleY);
    inputEdit->setFocusPolicy(Qt::ClickFocus);  // 只有点击时才能获取焦
    inputEdit->setStyleSheet(R"(
        QLineEdit {
            padding-left: 8px;
            color: #666666;
            border: 1px solid #ccc;  /*浅灰色边框 */
            border-radius: 4px;
            background-color: white; /*背景色 */
        }
    )");

    // 拿到单例键盘
    currentKeyboard = CustomKeyboard::instance(this);

    // 给每个 QLineEdit 注册一次偏移（如果你想要默认偏移都一样，就写同一个 QPoint）
    currentKeyboard->registerEdit(inputEdit,QPoint(-250*scaleX, 0));

    inputLayout->addWidget(inputLabel);
    inputLayout->addWidget(inputEdit);
    inputLayout->addStretch();
    inputLayout->setContentsMargins(50*scaleX, 0, 0, 0);

    mainLayout->addLayout(inputLayout);

    mainLayout->addSpacing(20*scaleY);

    // 按钮区域
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    resetBtn = new QPushButton(tr("重新校准"));
    resetBtn->setIcon(QIcon(":/image/icons8-refresh.png"));
    resetBtn->setIconSize(QSize(20*scaleX, 20*scaleY));
    resetBtn->setFixedSize(115*scaleX, 40*scaleY);
    saveBtn = new QPushButton(tr("保存"));
    saveBtn->setIcon(QIcon(":/image/icons8-save.png"));
    saveBtn->setIconSize(QSize(20*scaleX, 20*scaleY));
    saveBtn->setFixedSize(115*scaleX, 40*scaleY);

    resetBtn->setStyleSheet(R"(
    QPushButton {
        background-color: qlineargradient(
            x1:0, y1:0, x2:0, y2:1,
            stop:0 rgba(95, 169, 246, 180),
            stop:1 rgba(49, 122, 198, 180)
        );
        border: 1px solid rgba(163, 211, 255, 0.6); /* 半透明高光边框 */
        border-radius: 6px;
        color: white;
        font-weight: bold;
        font-size: 14px;
        padding: 2px 5px;
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

    saveBtn->setStyleSheet(R"(
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
    mainLayout->addSpacing(20*scaleY);
    buttonLayout->addWidget(resetBtn);
    buttonLayout->addStretch();
    buttonLayout->addWidget(saveBtn);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addSpacing(20*scaleY);

    // 连接按钮槽
    connect(saveBtn, &QPushButton::clicked, this, [this](){
        emit openmonitorwidget();
        this->close();
    });
    connect(resetBtn, &QPushButton::clicked, this, [this]() {
        inputEdit->clear();
    });
}

QString CalibrationDialog::getCalibrationValue() const
{
    return inputEdit->text();
}

void CalibrationDialog::changeEvent(QEvent *event){
    QWidget::changeEvent(event);
    if (event->type() == QEvent::LanguageChange) {
        // 系统自动发送的 LanguageChange
        setWindowTitle(tr("校准传感器"));
        title->setText(tr("校准传感器"));
        inputLabel->setText(tr("PA导管平均值"));
        inputEdit->setPlaceholderText(tr("输入校准值..."));
        resetBtn->setText(tr("重新校准"));
        saveBtn->setText(tr("保存"));
    }
}
