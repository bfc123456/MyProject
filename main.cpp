#include <QApplication>
#include <QSplashScreen>
#include <QScreen>
#include <QPainter>
#include <QLinearGradient>
#include <QLabel>
#include <QVBoxLayout>
#include <QFont>
#include <QMessageBox>
#include <QSettings>
#include <QTranslator>
#include <QDate>
#include <QThread>
#include "udpmanager.h"
#include "languagemanager.h"
#include "databasemanager.h"
#include "loginwindow.h"
#include "toucheventhandler.h"

LoginWindow* globalLoginWindowPointer = nullptr;
static QLabel *statusLabel = nullptr;

// 更新 Splash 上文字并立即刷新
void updateStatus(const QString& text) {
    if (statusLabel) {
        statusLabel->setText(text);
        QCoreApplication::processEvents();
    }
}

int main(int argc, char *argv[])
{
    // 触控支持
    QCoreApplication::setAttribute(Qt::AA_SynthesizeTouchForUnhandledMouseEvents);
    QCoreApplication::setAttribute(Qt::AA_SynthesizeMouseForUnhandledTouchEvents);

    QApplication a(argc, argv);
    a.installEventFilter(new TouchEventHandler());

    // --- Splash 界面搭建 ---
    QScreen *screen = QGuiApplication::primaryScreen();
    QSize screenSize = screen->availableGeometry().size();

    QPixmap pixmap(screenSize);
    pixmap.fill(Qt::transparent);
    {
        QPainter painter(&pixmap);
        QLinearGradient gradient(0,0,0,pixmap.height());
        gradient.setColorAt(0.0, QColor("#102b4e"));
        gradient.setColorAt(1.0, QColor("#1c3f5f"));
        painter.fillRect(pixmap.rect(), gradient);
    }

    QSplashScreen splash(pixmap);
    splash.setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    splash.setAttribute(Qt::WA_TranslucentBackground);
    splash.setFixedSize(screenSize);
    splash.showFullScreen();

    // Logo
    int logoW = screenSize.width() * 0.20;
    QLabel *logoLabel = new QLabel(&splash);
    logoLabel->setPixmap(
        QPixmap(":/image/logoimage.jpg").scaledToWidth(logoW, Qt::SmoothTransformation)
    );
    logoLabel->setAlignment(Qt::AlignCenter);
    logoLabel->setStyleSheet(
        "background-color: rgba(255,255,255,0.05);"
        "border-radius:12px; padding:10px;"
    );

    // 状态标签
    statusLabel = new QLabel(QObject::tr("系统初始化中…"), &splash);
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setStyleSheet("font-size:22px; color:white; font-weight:600;");

    // 版本/版权
    QLabel *versionLabel = new QLabel(QObject::tr("Version 1.0.0"), &splash);
    versionLabel->setAlignment(Qt::AlignCenter);
    versionLabel->setStyleSheet(
        QString("font-size:%1px; color:rgba(255,255,255,0.6);")
            .arg(screenSize.height() * 0.03)
    );
    QLabel *copyrightLabel = new QLabel(
        QObject::tr("© %1 芯联心 SI-Linking").arg(QDate::currentDate().year()),
        &splash
    );
    copyrightLabel->setAlignment(Qt::AlignCenter);
    copyrightLabel->setStyleSheet(
        QString("font-size:%1px; color:rgba(255,255,255,0.4);")
            .arg(screenSize.height() * 0.013)
    );

    // 布局
    QVBoxLayout *vlay = new QVBoxLayout(&splash);
    int margin = screenSize.width() * 0.02;
    vlay->setContentsMargins(margin, margin, margin, margin);
    vlay->setSpacing(screenSize.height() * 0.015);
    vlay->addStretch();
    vlay->addWidget(logoLabel, 0, Qt::AlignHCenter);
    vlay->addSpacing(20);
    vlay->addWidget(statusLabel);
    vlay->addSpacing(10);
    vlay->addWidget(versionLabel);
    vlay->addWidget(copyrightLabel);
    vlay->addStretch();
    splash.show();

    // === 真正初始化流程 ===

    // 1. 加载本地设置（语言等）
    updateStatus(QObject::tr("加载本地配置…"));
    QSettings settings("MyCompany", "MyApp");
    QString language = settings.value("language", "zh_CN").toString();

    // 2. 初始化网络模块
    updateStatus(QObject::tr("初始化网络模块…"));
    UdpManager udpManager;
    if (!udpManager.startListening(5005)) {
        QMessageBox::critical(nullptr,
                              QObject::tr("错误"),
                              QObject::tr("UDP 监听失败！"));
        return -1;
    }

    // 3. 连接数据库
    updateStatus(QObject::tr("连接数据库…"));
    auto &dbMgr = DatabaseManager::instance(
        "E:/software_personal/personal_program/MyProject/MyDatabase.db"
    );
    if (!dbMgr.openDatabase()) {
        QMessageBox::critical(nullptr,
                              QObject::tr("错误"),
                              QObject::tr("数据库连接失败！"));
        return -1;
    }

    // 4. 安装翻译器
    updateStatus(QObject::tr("加载翻译文件…"));
    {
        QTranslator translator;
        if (translator.load(":/translations/app_" + language + ".qm")) {
            a.installTranslator(&translator);
        }
    }

    // 5. 准备主界面
    updateStatus(QObject::tr("准备主界面…"));
    QThread::msleep(300);

    // 展示登录窗口
    splash.close();
    globalLoginWindowPointer = new LoginWindow();
    globalLoginWindowPointer->setFixedSize(1024, 600);
    globalLoginWindowPointer->setAttribute(Qt::WA_AcceptTouchEvents);
    globalLoginWindowPointer->show();

    int ret = a.exec();
    delete globalLoginWindowPointer;
    return ret;
}
