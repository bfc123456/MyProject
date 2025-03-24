#include <QApplication>
#include <QSplashScreen>
#include <QProgressBar>
#include <QTimer>
#include <QScreen>
#include <QTranslator>
#include <QSettings>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //加载对应语言
    // 读取上次存储的语言，默认 "zh_CN"
    QSettings settings("MyCompany", "MyApp");
    QString language = settings.value("language", "zh_CN").toString();

    //加载翻译文件
    QTranslator translator;
    if (translator.load(":/translations/app_" + language + ".qm")) {
        a.installTranslator(&translator);
    }

    // 创建启动画面
    QSplashScreen splash;
    splash.setWindowFlag(Qt::FramelessWindowHint);
    splash.setWindowFlag(Qt::WindowStaysOnTopHint);
    splash.setFixedSize(500, 300);
    splash.setStyleSheet("background-color: black;");

    // 设置轻微圆角
    QRegion region(0, 0, splash.width(), splash.height(), QRegion::Rectangle);
    QPainterPath path;
    int cornerRadius = 8; //  控制四角弧度
    path.addRoundedRect(0, 0, splash.width(), splash.height(), cornerRadius, cornerRadius);
    region = QRegion(path.toFillPolygon().toPolygon());
    splash.setMask(region); // 应用裁剪

    // 启动画面位置居中
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->availableGeometry();
    int x = (screenGeometry.width() - splash.width()) / 2;
    int y = (screenGeometry.height() - splash.height()) / 2;
    splash.move(x, y);

    // 显示软件版本号
    QLabel versionLabel("Version 1.0.0", &splash);
    versionLabel.setStyleSheet("font-size: 16px; color: white;");
    versionLabel.setAlignment(Qt::AlignCenter);
    versionLabel.adjustSize();
    int labelX = (splash.width() - versionLabel.width()) / 2;
    int labelY = splash.height() - versionLabel.height() - 40;
    versionLabel.move(labelX, labelY);

    // "正在加载..." 文字
    QLabel label(("Loading, please wait..."), &splash);
    label.setStyleSheet("font-size: 18px; color: white; font-weight: bold;");
    label.setAlignment(Qt::AlignCenter);
    int centerY = (splash.height() - 50) / 2;
    label.setGeometry(0, centerY, splash.width(), 50);

    // 进度条
    QProgressBar progressBar(&splash);
    progressBar.setGeometry(50, 200, 400, 30); // 进度条大小
    progressBar.setStyleSheet(
        "QProgressBar {"
        "    border: 2px solid white;"
        "    border-radius: 5px;"
        "    background: #444444;"  // 进度条背景色（深灰）
        "}"
        "QProgressBar::chunk {"
        "    border-radius: 5px;"
        "    background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, "
        "        stop:0 #00FF00, stop:1 #007BFF);"
        "}"  // 渐变色：从绿色 (#00FF00) 到蓝色 (#007BFF)
    );
    progressBar.setRange(0, 100);
    progressBar.setValue(0);

    // **让进度条内部显示居中的百分比**
    progressBar.setFormat("%p%");  // 显示进度条百分比
    progressBar.setAlignment(Qt::AlignCenter);  // 让文本居中
    progressBar.setTextVisible(true);  // 确保文本可见

    splash.show();

    // 创建 MainWindow
    MainWindow w;
    w.setFixedSize(1024, 600);

    // QTimer 更新进度条
    QTimer timer;
    int progress = 0;
    QObject::connect(&timer, &QTimer::timeout, [&]() {
        progress += 5; // 每次增加 5%
        progressBar.setValue(progress);

        if (progress >= 100) {
            timer.stop();
            splash.close();
            w.show();
        }
    });

    timer.start(100); // 每 100ms 更新一次进度条

    return a.exec();
}

