#include "calibrationdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFont>
#include <QMessageBox>
#include "customkeyboard.h"
#include <QGuiApplication>
#include <QScreen>
#include <QGraphicsDropShadowEffect>
#include <QPainterPath>
#include <QRegion>

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

    setFixedSize(400*scaleX, 280*scaleY);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setObjectName("CalibrationDialog");

    setStyleSheet(R"(
    #CalibrationDialog{
        background: transparent;
    }
    )");

    QVBoxLayout* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setAlignment(Qt::AlignCenter);

    // 卡片容器
    QFrame* card = new QFrame(this);
    card->setObjectName("card");
    card->setFixedSize(400*scaleX, 280*scaleY);

    const int r = 16; // 圆角半径，和 QSS 里一致

    QPainterPath path;
    path.addRoundedRect(card->rect(), r, r);
    setMask(QRegion(path.toFillPolygon().toPolygon()));

    card->setStyleSheet(R"(
    #card{
        background-color: #262A33;
        border-radius: 16px;
        border: 1px solid rgba(255,255,255,90);
    }
    QLabel{
        color: rgba(255,255,255,220);
    }
    )");

    auto *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(26);
    shadow->setOffset(0, 10);
    shadow->setColor(QColor(0, 0, 0, 180));
    card->setGraphicsEffect(shadow);

    rootLayout->addWidget(card);

    QVBoxLayout* mainLayout = new QVBoxLayout(card);
    mainLayout->setContentsMargins(
        26*scaleX, 22*scaleY,
        26*scaleX, 24*scaleY
    );
    mainLayout->setSpacing(14*scaleY);

    //创建关闭按钮
//    QPushButton *closeButton = new QPushButton(this);
//    closeButton->setIcon(QIcon(":/image/icons-close.png"));
//    closeButton->setIconSize(QSize(20 * scaleX, 20 * scaleX));
    closeButton = new QPushButton(card);
    closeButton->setObjectName("closeBtn");
    closeButton->setIcon(QIcon(":/image/icons-close.png"));
    closeButton->setIconSize(QSize(20*scaleX,20*scaleX));
    closeButton->setFixedSize(28*scaleX, 28*scaleX);
    closeButton->setCursor(Qt::PointingHandCursor);

    closeButton->setStyleSheet(R"(
    #closeBtn{
        background: transparent;
        border: none;
        border-radius: 14px;
        color: rgba(255,255,255,180);
        font-size: 14px;
        font-weight: 700;
    }
    #closeBtn:hover{
        background: rgba(255,255,255,40);
        color: white;
    }
    #closeBtn:pressed{
        background: rgba(0,0,0,70);
    }
    )");


    //创建按钮布局，用于设置关闭按钮的显示位置
    QHBoxLayout *headerLayout = new QHBoxLayout();
    headerLayout->setContentsMargins(0,0,0,0);
    headerLayout->addWidget(closeButton,0,Qt::AlignRight);
    mainLayout->addLayout(headerLayout);

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
    QLabel* pulseValue = new QLabel("94");
    pulseValue->setFixedWidth(80*scaleX);
    pulseValue->setAlignment(Qt::AlignCenter);
    QLabel* unit = new QLabel("mmHg");
    unit->setFixedWidth(120*scaleX);
    unit->setAlignment(Qt::AlignCenter);
    bpValue->setStyleSheet("font-size: 28px; font-weight: 700; color: white;background-color: transparent;");
    pulseValue->setStyleSheet("font-size: 28px; font-weight: 700; color: white;background-color: transparent;");
    unit->setStyleSheet("font-size: 12px; color: rgba(255,255,255,160);background-color: transparent;");


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
    QLineEdit{
        padding-left: 10px;
        color: rgba(255,255,255,230);
        border: 1px solid rgba(255,255,255,70);
        border-radius: 8px;
        background-color: rgba(255,255,255,18);
    }
    QLineEdit:focus{
        border: 1px solid rgba(30,140,255,200);
        background-color: rgba(255,255,255,24);
    }
    )");


//    // 拿到单例键盘
//    currentKeyboard = CustomKeyboard::instance(this);

//    // 给每个 QLineEdit 注册一次偏移（如果你想要默认偏移都一样，就写同一个 QPoint）
//    currentKeyboard->registerEdit(inputEdit,QPoint(-250*scaleX, 0));

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

    QString btnStyle = R"(
    QPushButton{
        background-color: #1E8CFF;
        border: none;
        border-radius: 10px;
        color: white;
        font-weight: 700;
        font-size: 14px;
        padding: 6px 18px;
    }
    QPushButton:hover{
        background-color: #3C9DFF;
    }
    QPushButton:pressed{
        background-color: #1673D2;
    }
    )";
    resetBtn->setStyleSheet(btnStyle);
    saveBtn->setStyleSheet(btnStyle);


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
    connect(closeButton, &QPushButton::clicked, this, &CalibrationDialog::close);

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
