
#include "exitconfirmdialog.h"
#include <QIcon>
#include <QGraphicsDropShadowEffect>
#include <QGuiApplication>
#include <QScreen>
#include <QFrame>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

ExitConfirmDialog::ExitConfirmDialog(QWidget *parent)
    : CloseOnlyWindow(parent)
{
    // ====== 屏幕缩放 ======
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int screenWidth  = screenGeometry.width();
    int screenHeight = screenGeometry.height();

    scaleX = (float)screenWidth  / 1024;
    scaleY = (float)screenHeight / 600;

    // ====== 透明外层 + 无边框 ======
    setFixedSize(520 * scaleX, 300 * scaleY);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setObjectName("ExitConfirmDialog");
    setStyleSheet(R"(
        #ExitConfirmDialog { background: transparent; }
    )");

    // ====== Root：居中放卡片 ======
    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setAlignment(Qt::AlignCenter);

    // ====== Card：主体容器 ======
    QFrame *card = new QFrame(this);
    card->setObjectName("card");
    card->setFixedSize(size());
    card->setStyleSheet(R"(
        #card{
            background-color: #262A33;
            border: 1px solid rgba(255,255,255,90);
            border-radius: 16px;
        }
        #card QLabel{
            background: transparent;
            color: rgba(255,255,255,230);
        }
    )");

    auto *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(26);
    shadow->setOffset(0, 10);
    shadow->setColor(QColor(0,0,0,180));
    card->setGraphicsEffect(shadow);

    root->addWidget(card);

    // ====== 卡片内部主布局 ======
    QVBoxLayout *mainLayout = new QVBoxLayout(card);
    mainLayout->setContentsMargins(28 * scaleX, 22 * scaleY, 28 * scaleX, 22 * scaleY);
    mainLayout->setSpacing(16 * scaleY);

    // ====== 顶栏：标题居中 + X 透明按钮（无轮廓） ======
    QLabel *titleLabel = new QLabel(tr("提示"), card);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(
        "font-size: 18px;"
        "font-weight: 700;"
        "letter-spacing: 1px;"
        "color: rgba(255,255,255,210);"
    );

    QPushButton *closeButton = new QPushButton(card);
    closeButton->setObjectName("closeBtn");
    closeButton->setText("X");   // ARM 更稳（不要用 ✕）
    closeButton->setFixedSize(30 * scaleX, 30 * scaleY);
    closeButton->setCursor(Qt::PointingHandCursor);

    // ✅ 透明 + 无轮廓 + hover/focus/pressed 都不画圈
    closeButton->setStyleSheet(R"(
        QPushButton{
            background: transparent;
            border: none;
            outline: none;
            color: rgba(255,255,255,170);
            font-size: 14px;
            font-weight: 800;
        }
        QPushButton:hover{
            background: transparent;
            border: none;
            outline: none;
            color: rgba(255,255,255,255);
        }
        QPushButton:pressed{
            background: transparent;
            border: none;
            outline: none;
            color: rgba(255,255,255,140);
        }
        QPushButton:focus{
            background: transparent;
            border: none;
            outline: none;
        }
    )");

    connect(closeButton, &QPushButton::clicked, this, &QDialog::reject);

    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->setContentsMargins(0, 0, 0, 0);

    // 左侧占位，保证标题居中视觉更稳
    QWidget *leftSpacer = new QWidget(card);
    leftSpacer->setFixedWidth(closeButton->sizeHint().width());

    topLayout->addWidget(leftSpacer);
    topLayout->addStretch();
    topLayout->addWidget(titleLabel);
    topLayout->addStretch();
    topLayout->addWidget(closeButton);

    mainLayout->addLayout(topLayout);

    // ====== 提示文本（居中） ======
    messageLabel = new QLabel(tr("确定退出此界面"), card);
    messageLabel->setAlignment(Qt::AlignCenter);   // ✅ 居中
    messageLabel->setWordWrap(true);
    messageLabel->setStyleSheet(
        "font-size: 16px;"
        "color: rgba(255,255,255,210);"
    );

    mainLayout->addSpacing(6 * scaleY);
    mainLayout->addWidget(messageLabel);
    mainLayout->addSpacing(12 * scaleY);

    // ====== 底部按钮（有间隔，规整） ======
    shutdownButton = new QPushButton(tr("关闭系统"), card);
    shutdownButton->setIcon(QIcon(":/icons/shutdown.png"));
    shutdownButton->setFixedSize(170 * scaleX, 46 * scaleY);
    shutdownButton->setCursor(Qt::PointingHandCursor);

    returnButton = new QPushButton(tr("返回主界面"), card);
    returnButton->setIcon(QIcon(":/icons/home.png"));
    returnButton->setFixedSize(170 * scaleX, 46 * scaleY);
    returnButton->setCursor(Qt::PointingHandCursor);

    QString primaryBtn = R"(
        QPushButton{
            background-color: #1E8CFF;
            border: none;
            border-radius: 12px;
            color: white;
            font-weight: 700;
            font-size: 14px;
            padding: 6px 16px;
        }
        QPushButton:hover{ background-color: #3C9DFF; }
        QPushButton:pressed{ background-color: #1673D2; }
    )";

    QString dangerBtn = R"(
        QPushButton{
            background-color: rgba(255, 80, 80, 0.22);
            border: 1px solid rgba(255, 120, 120, 0.45);
            border-radius: 12px;
            color: rgba(255,255,255,230);
            font-weight: 700;
            font-size: 14px;
            padding: 6px 16px;
        }
        QPushButton:hover{ background-color: rgba(255, 80, 80, 0.30); }
        QPushButton:pressed{ background-color: rgba(255, 80, 80, 0.38); }
    )";

    returnButton->setStyleSheet(primaryBtn);
    shutdownButton->setStyleSheet(dangerBtn);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setContentsMargins(0, 0, 0, 0);

    buttonLayout->addStretch();
    buttonLayout->addWidget(returnButton);

    // ✅ 按钮中间间隔（你要的效果）
    buttonLayout->addSpacing(24 * scaleX);

    buttonLayout->addWidget(shutdownButton);
    buttonLayout->addStretch();

    mainLayout->addLayout(buttonLayout);

    // ====== 信号连接（保持原功能） ======
    connect(shutdownButton, &QPushButton::clicked, this, &ExitConfirmDialog::shutdownSystem);
    connect(returnButton, &QPushButton::clicked, this, &ExitConfirmDialog::returnToMain);
}



ExitConfirmDialog::~ExitConfirmDialog(){

}

void ExitConfirmDialog::changeEvent(QEvent *event) {
    QWidget::changeEvent(event);
    if (event->type() == QEvent::LanguageChange) {
        messageLabel->setText(tr("确定退出此界面"));
        shutdownButton->setText(tr("关闭系统"));
        returnButton->setText(tr("返回主界面"));
    }
}
