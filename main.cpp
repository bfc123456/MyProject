#include <QApplication>
#include <QSplashScreen>
#include <QProgressBar>
#include <QTimer>
#include <QScreen>
#include <QTranslator>
#include <QSettings>
#include "mainwindow.h"
#include "loginwindow.h"
#include "implantdatabase.h"
#include "toucheventhandler.h"
#include "Global.h"

LoginWindow* globalLoginWindowPointer = nullptr;  // 定义并初始化全局指针，注意全局指针的内存释放，防止内存泄漏！！！

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

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

    //创建数据库并验证数据库是否连接成功
    // 创建数据库连接
    ImplantDatabase db("D:/software_install/sqlite/implants.db");

    // 步骤 1：创建渐变 QPixmap 作为启动背景
    QPixmap pixmap(500, 300);
    pixmap.fill(Qt::transparent);  // 保证透明背景

    QPainter painter(&pixmap);
    QLinearGradient gradient(0, 0, 0, pixmap.height());
    gradient.setColorAt(0.0, QColor("#0f2243"));  // 顶部深蓝
    gradient.setColorAt(1.0, QColor("#1a2d67"));  // 底部亮蓝
    painter.fillRect(pixmap.rect(), gradient);
    painter.end();

    // 步骤 2：创建带渐变背景的 splash
    QSplashScreen splash(pixmap);
    splash.setWindowFlag(Qt::FramelessWindowHint);
    splash.setWindowFlag(Qt::WindowStaysOnTopHint);
    splash.setFixedSize(500, 300);


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
        "    background-color: rgba(255, 255, 255, 0.1);"  // 浅透明背景
        "    border: 1px solid #1E90FF;"  // 深蓝边框
        "    border-radius: 8px;"
        "    text-align: center;"
        "    color: white;"
        "    font-weight: bold;"
        "}"
        "QProgressBar::chunk {"
        "    border-radius: 8px;"
        "    background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, "
        "        stop:0 #4facfe, stop:1 #00f2fe);"  // 更高端蓝系渐变
        "}"
    );

    progressBar.setRange(0, 100);
    progressBar.setValue(0);

    // **让进度条内部显示居中的百分比**
    progressBar.setFormat("%p%");  // 显示进度条百分比
    progressBar.setAlignment(Qt::AlignCenter);  // 让文本居中
    progressBar.setTextVisible(true);  // 确保文本可见

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
        progressBar.setValue(progress);

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



