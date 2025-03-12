#include <QApplication>
#include <QSplashScreen>
#include <QProgressBar>
#include <QTimer>
#include <QLabel>
#include <QScreen>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // **1. 创建启动画面（QSplashScreen）**
    QSplashScreen splash;
    splash.setWindowFlag(Qt::FramelessWindowHint); // 无边框
    splash.setWindowFlag(Qt::WindowStaysOnTopHint); // 保持最前
    splash.setFixedSize(500, 300); // 设置启动窗口大小
    splash.setStyleSheet("background-color: black;"); // 黑色背景

    // **2. 计算启动画面居中位置**
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->availableGeometry();
    int x = (screenGeometry.width() - splash.width()) / 2;
    int y = (screenGeometry.height() - splash.height()) / 2;
    splash.move(x, y);

    // **3. 添加软件版本号**
    QLabel versionLabel("Version 1.0.0", &splash);
    versionLabel.setStyleSheet("font-size: 16px; color: white;");
    versionLabel.setAlignment(Qt::AlignCenter);
    versionLabel.adjustSize();

    // **4. 添加数据库版本号**
    QLabel dbVersionLabel("Database Version: 2.3.4", &splash);
    dbVersionLabel.setStyleSheet("font-size: 14px; color: white;");
    dbVersionLabel.setAlignment(Qt::AlignCenter);
    dbVersionLabel.adjustSize();

    // **5. 计算版本号标签的位置**
    int labelX = (splash.width() - versionLabel.width()) / 2;
    int labelY = splash.height() - versionLabel.height() - 40;
    versionLabel.move(labelX, labelY);

    int dbLabelX = (splash.width() - dbVersionLabel.width()) / 2;
    int dbLabelY = splash.height() - dbVersionLabel.height() - 20;
    dbVersionLabel.move(dbLabelX, dbLabelY);

    // **6. 添加 "正在加载..." 文字**
    QLabel label("正在加载，请稍候...", &splash);
    label.setStyleSheet("font-size: 18px; color: white; font-weight: bold;");
    label.setAlignment(Qt::AlignCenter);

    // 计算垂直居中位置
    int centerY = (splash.height() - 50) / 2;  // 50 是 QLabel 的高度
    label.setGeometry(0, centerY, splash.width(), 50);


    // **7. 添加进度条**
    QProgressBar progressBar(&splash);
    progressBar.setGeometry(50, 200, 400, 30); // 进度条大小
    progressBar.setStyleSheet(
        "QProgressBar { border: 2px solid white; border-radius: 5px; background: gray; }"
        "QProgressBar::chunk { background-color: #007BFF; width: 10px; }"); // 进度条颜色
    progressBar.setRange(0, 100); // 设置范围
    progressBar.setValue(0); // 初始值 0

    splash.show(); // 显示启动界面

    // **8. 创建 MainWindow 实例，避免 QTimer 作用域销毁**
    MainWindow *w = new MainWindow();
    w->setFixedSize(1024, 600);

    // **9. 使用 QTimer 模拟加载进度**
    QTimer timer;
    int progress = 0;
    QObject::connect(&timer, &QTimer::timeout, [&]() {
        progress += 5; // 每次增加 5%
        progressBar.setValue(progress);

        if (progress >= 100) { // 100% 关闭启动画面，显示主窗口
            timer.stop();
            splash.close();
            w->show();
        }
    });

    timer.start(100); // 每 100ms 更新一次进度条

    return a.exec();
}
