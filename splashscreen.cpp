#include "splashscreen.h"
#include <QPixmap>
#include <QPainter>
#include <QLinearGradient>
#include <QVBoxLayout>
#include <QCoreApplication>
#include <QThread>
#include <QSettings>

SplashScreen::SplashScreen(QScreen *screen, QWidget *parent) : QSplashScreen(parent) {
    // 设置屏幕大小和背景
    QRect fullRect = screen->geometry();
    setGeometry(fullRect);

    // 初始化界面元素
    initUI(screen, fullRect);
}

void SplashScreen::UpdateStatus(const QString& text, int progress) {
    if (m_pStatusLabel) {
        m_pStatusLabel->setText(text);
    }
    if (m_pProgressBar) {
        m_pProgressBar->setValue(progress);
    }
    QCoreApplication::processEvents();
}

void SplashScreen::SetProgressMax(int max) {
    if (m_pProgressBar) {
        m_pProgressBar->setMaximum(max);
    }
}

void SplashScreen::initUI(QScreen *screen, const QRect &fullRect)
{
    // ===== 背景渐变 =====
    QPixmap pixmap(fullRect.size());
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);

    QLinearGradient gradient(0, 0, 0, pixmap.height());
    gradient.setColorAt(0.0, QColor("#103c5b"));   // 深蓝
    gradient.setColorAt(1.0, QColor("#1e4e76"));   // 柔和蓝
    painter.fillRect(pixmap.rect(), gradient);

    setPixmap(pixmap);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);

    const int iSw = screen->geometry().width();
    const int iSh = screen->geometry().height();

    // ===== 中间“卡片容器” =====
    QFrame *pCardFrame = new QFrame(this);
    pCardFrame->setObjectName("splashCard");
    pCardFrame->setFixedWidth(iSw * 0.45);
    pCardFrame->setStyleSheet(
        "#splashCard {"
        "   background-color: rgba(255,255,255,40);"   // 半透明“毛玻璃”效果
        "   border-radius: 20px;"
        "   border: 1px solid rgba(255,255,255,80);"
        "}"
    );

    QVBoxLayout *pCardLayout = new QVBoxLayout(pCardFrame);
    pCardLayout->setContentsMargins(36, 30, 36, 24);
    pCardLayout->setSpacing(12);

    // ===== Logo =====
    QLabel *pLogoLabel = new QLabel(pCardFrame);
    QPixmap logoPix(":/image/logoimage.png");
    pLogoLabel->setPixmap(
        logoPix.scaledToWidth(iSw * 0.18, Qt::SmoothTransformation)
    );
    pLogoLabel->setAlignment(Qt::AlignCenter);

    // ===== 状态标签（放到卡片里面，做副标题） =====
    m_pStatusLabel = new QLabel(tr("系统初始化中…"), pCardFrame);
    m_pStatusLabel->setAlignment(Qt::AlignCenter);
    m_pStatusLabel->setStyleSheet(
        "font-size:16px;"
        "color: rgba(255,255,255,220);"
        "font-weight:500;"
    );

    // ===== 版本信息 =====
    QLabel *versionLabel = new QLabel(tr("Version 1.0.0"), pCardFrame);
    versionLabel->setAlignment(Qt::AlignCenter);
    versionLabel->setStyleSheet(
        QString("font-size:%1px; color:rgba(255,255,255,200);")
            .arg(iSh * 0.018)
    );

    pCardLayout->addWidget(pLogoLabel);
    pCardLayout->addSpacing(6);
    pCardLayout->addWidget(m_pStatusLabel);
    pCardLayout->addWidget(versionLabel);

    // ===== 进度条（在卡片下面，细一点，更现代） =====
    m_pProgressBar = new QProgressBar(this);
    m_pProgressBar->setRange(0, 100);
    m_pProgressBar->setValue(0);
    m_pProgressBar->setTextVisible(false);
    m_pProgressBar->setFixedWidth(iSw * 0.45);
    m_pProgressBar->setObjectName("splashProgress");

    m_pProgressBar->setStyleSheet(
        "#splashProgress {"
        "   border: none;"
        "   background-color: rgba(255,255,255,40);"
        "   border-radius: 6px;"
        "   height: 10px;"
        "}"
        "#splashProgress::chunk {"
        "   border-radius: 6px;"
        "   background: qlineargradient("
        "       x1:0, y1:0, x2:1, y2:0,"
        "       stop:0 #55aaff,"
        "       stop:0.5 #00d27a,"
        "       stop:1 #a3ff3f);"
        "}"
    );

    // ===== Credits 信息（放底部，小而淡） =====
    QLabel *pCreditsLabel = new QLabel(tr("Icons by Icons8"), this);
    pCreditsLabel->setAlignment(Qt::AlignCenter);
    pCreditsLabel->setStyleSheet(
        "font-size:12px;"
        "color: rgba(255,255,255,140);"
    );

    // ===== 外层布局：整体垂直居中 =====
    QVBoxLayout *vlay = new QVBoxLayout(this);
    vlay->setContentsMargins(0, iSh * 0.12, 0, iSh * 0.08);
    vlay->setSpacing(18);

    vlay->addStretch();
    vlay->addWidget(pCardFrame, 0, Qt::AlignHCenter);
    vlay->addSpacing(12);
    vlay->addWidget(m_pProgressBar, 0, Qt::AlignHCenter);
    vlay->addSpacing(10);
    vlay->addWidget(pCreditsLabel, 0, Qt::AlignHCenter);
    vlay->addStretch();

    setLayout(vlay);
}

