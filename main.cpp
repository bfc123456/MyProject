#include <QApplication>
#include <QSplashScreen>
#include <QProgressBar>
#include <QTimer>
#include <QScreen>
#include <QTranslator>
#include <QSettings>
#include "mainwindow.h"
#include "loginwindow.h"
#include "toucheventhandler.h"
#include "global.h"
#include "databasemanager.h"
#include "languagemanager.h"

LoginWindow* globalLoginWindowPointer = nullptr;  // 定义并初始化全局指针，注意全局指针的内存释放，防止内存泄漏！！！

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    LanguageManager::instance(); //初始化语言环境

    //创建数据库
    DatabaseManager db("E:/software_personal/personal_program/MyProject/MyDatabase.db");

    //判断数据库是否正确连接
    if (!db.openDatabase()) {
        return -1;  // 打不开就退出
    }

    // 设置全局字体为黑体
    QFont font("SimHei");
    a.setFont(font);

    //加载对应语言
    // 读取上次存储的语言，默认 "zh_CN"
    QSettings settings("MyCompany", "MyApp");
    QString language = settings.value("language", "zh_CN").toString();

    //加载翻译文件
    QTranslator translator;
    if (translator.load(":/translations/app_" + language + ".qm")) {
        a.installTranslator(&translator);
    }

    // 创建渐变 QPixmap 作为启动背景
    QPixmap pixmap(500, 300);
    pixmap.fill(Qt::transparent);  // 保证透明背景

    QPainter painter(&pixmap);
    QLinearGradient gradient(0, 0, 0, pixmap.height());
    gradient.setColorAt(0.0, QColor("#0f2243"));  // 顶部深蓝
    gradient.setColorAt(1.0, QColor("#1a2d67"));  // 底部亮蓝
    painter.fillRect(pixmap.rect(), gradient);
    painter.end();

    // 创建带渐变背景的 splash
    QSplashScreen splash(pixmap);
    splash.setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    splash.setAttribute(Qt::WA_TranslucentBackground);
    splash.setFixedSize(500, 300);

    QLabel *logoLabel = new QLabel(&splash);
    logoLabel->setPixmap(QPixmap(":/image/logoimage.jpg")
                         .scaledToWidth(180, Qt::SmoothTransformation));
    logoLabel->setAlignment(Qt::AlignCenter);

    QLabel *tipLabel = new QLabel(QObject::tr("Loading, please wait..."), &splash);
    tipLabel->setAlignment(Qt::AlignCenter);
    tipLabel->setStyleSheet("font-size:18px; color:rgba(255,255,255,0.9);");

    QProgressBar *progressBar = new QProgressBar(&splash);
    progressBar->setRange(0,100);
    progressBar->setFormat("%p%");
    progressBar->setFixedWidth(380);
    progressBar->setAlignment(Qt::AlignCenter);
    progressBar->setTextVisible(true);
    // 让它自动横向拉伸，高度固定
    progressBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    progressBar->setFixedHeight(16);
    progressBar->setStyleSheet(R"(
                               QProgressBar {
                               background: rgba(255,255,255,0.08);
                               border: none; border-radius: 8px;
                               color: rgba(255,255,255,0.9);
                               font-weight: bold;
                               }
                               QProgressBar::chunk {
                               border-radius: 8px;
                               background: qlineargradient(
                               x1:0, y1:0, x2:1, y2:0,
                               stop:0 #57C7FF, stop:1 #00A3FF
                               );
                               }
                               )");

    QLabel *versionLabel = new QLabel(QObject::tr("Version 1.0.0"), &splash);
    versionLabel->setAlignment(Qt::AlignCenter);
    versionLabel->setStyleSheet("font-size:12px; color:rgba(255,255,255,0.6);");

    // 布局
    auto *vlay = new QVBoxLayout(&splash);
    vlay->setContentsMargins(20,20,20,20);
    vlay->setSpacing(12);
    vlay->addStretch();                                  // 上方留白
    vlay->addWidget(logoLabel,    0, Qt::AlignHCenter);
    vlay->addWidget(tipLabel);
    vlay->addWidget(progressBar, 0 , Qt::AlignHCenter);
    vlay->addWidget(versionLabel);
    vlay->addStretch();                                  // 下方留白

    // 中心屏幕
    QRect geo = QGuiApplication::primaryScreen()->availableGeometry();
    splash.move((geo.width()-splash.width())/2,
                (geo.height()-splash.height())/2);
    splash.show();

    // 创建 MainWindow

    globalLoginWindowPointer = new LoginWindow();
    globalLoginWindowPointer->setFixedSize(1024, 600);
    globalLoginWindowPointer->setAttribute(Qt::WA_AcceptTouchEvents);

    // QTimer 更新进度条
    QTimer timer;
    int progress = 0;
    QObject::connect(&timer, &QTimer::timeout, [&]() {
        progress += 5; // 每次增加 5%
        progressBar->setValue(progress);
        if (progress >= 100) {
            timer.stop();
            splash.close();
            globalLoginWindowPointer->show();
        }
    });

    timer.start(100); // 每 100ms 更新一次进度条

    int result = a.exec();

    // 在程序结束时删除动态分配的对象
    delete globalLoginWindowPointer;
    globalLoginWindowPointer = nullptr;  // 避免悬空指针

    return result;
}



