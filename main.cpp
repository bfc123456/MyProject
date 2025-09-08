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
#include "MedicalLogger.h"
#include "UdpManager.h"
#include "LanguageManager.h"
#include "DatabaseManager.h"
#include "multiuserloginwindow.h"
#include "TouchEventHandler.h"
#include "Global.h"
#include "DeviceAcquisitionWorker.h"
#include "MeasurementDataProcessor.h"
#include "MeasurementData.h"
#include "updatemanager.h"

//版本宏定义
#define APP_VERSION "1.0.0"
//现成超时时间
#define THREAD_WAIT_TIMEOUT 3000

//显示加载状态
static QLabel *statusLabel = nullptr;

// 更新 Splash 上文字并立即刷新
void updateStatus(const QString& text) {
    if (statusLabel) {
        statusLabel->setText(text);
        QCoreApplication::processEvents();
    }
    //记录初始化步骤日志
//     MedicalLogger::instance().logInfo(text.toStdString());  // 记录初始化过程
}

int main(int argc, char *argv[])
{
    //触控支持
    QCoreApplication::setAttribute(Qt::AA_SynthesizeTouchForUnhandledMouseEvents);
    QCoreApplication::setAttribute(Qt::AA_SynthesizeMouseForUnhandledTouchEvents);

    QApplication a(argc, argv);
    a.installEventFilter(new TouchEventHandler());
    a.setApplicationName("MeasurementtApp");
    qRegisterMetaType<MeasurementData>("MeasurementData");

    MedicalLogger::instance()->init();   //启动日志记录
    qInstallMessageHandler(MedicalLogger::qtMessageHandler);  // 设置自定义的 Qt 消息处理器

    //在日志中记录初始化信息
    updateStatus(QObject::tr("系统初始化..."));
    MedicalLogger::instance()->writeLog(
        "Configuration",
        MedicalLogger::LOG_INFO,
        "System Initialization Started",
        " ",   // 操作员 ID
        "System"
    );

    //当应用退出时，清理日志文件
    QObject::connect(&a,&QCoreApplication::aboutToQuit,[](){
       //清理日志资源
        MedicalLogger::instance()->shutdown();
    });

    //Splash 界面搭建
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect fullRect = screen->geometry();        // 包含任务栏
    QSize screenSize = fullRect.size();

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
    splash.setGeometry(fullRect);

    //Logo 显示
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

    //状态标签 显示自启动流程
    statusLabel = new QLabel(QObject::tr("系统初始化中…"), &splash);
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setStyleSheet("font-size:22px; color:white; font-weight:600;");

    //版本/版权 显示
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

    //加载界面布局
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
    splash.showFullScreen();

    // === 真正初始化流程 ===

    // 1. 加载本地设置
    updateStatus(QObject::tr("加载本地配置…"));
    QSettings settings("MyCompany", "MyApp");
    QString language = settings.value("language", "zh_CN").toString();
    MedicalLogger::instance()->writeLog(
        "Configuration",           // 日志模块/操作类别：配置相关操作
        MedicalLogger::LOG_INFO,   // 日志级别：信息
        "System configuration loaded successfully", // 日志内容
        "UnknownOperator",         // 操作员 ID（如果没有登录就使用占位符）
        "System"                   // 传感器/子系统 ID（这里是系统操作，填写 "System"）
    );
    QCoreApplication::processEvents();
    QThread::msleep(300);

    // 2. 检查更新
    UpdateManager um;
    QString updateMsg;
    um.applyPendingIfAny(&updateMsg);
    //想在启动后给出提示
    if (!updateMsg.isEmpty()) {
        updateStatus(QObject::tr("检测到软件更新: %1").arg(updateMsg));
        MedicalLogger::instance()->writeLog(
            "Configuration",                            // 模块
            MedicalLogger::LOG_AUDIT,            // 审计日志，重要操作
            QString("Pending update applied: %1").arg(updateMsg), // 日志内容
            " ",                   // 操作员ID（此处无登录，用 Unknown）
            "System"                             // 传感器/子系统 ID
        );
        CustomMessageBox(
            nullptr,
            QObject::tr("软件更新"),
            QObject::tr("检测到更新并已应用: %1").arg(updateMsg),
            { QObject::tr("确定") },
            380
        ).exec();
    } else {
        updateStatus(QObject::tr("未检测到挂起的软件更新"));
        MedicalLogger::instance()->writeLog(
            "Configuration",
            MedicalLogger::LOG_INFO,
            "No pending update detected",
            " ",
            "System"
        );
    }

    // 3. 读取信号强度（默认 70）
    int savedStrength = settings.value("system/signalStrength", 70).toInt();
    a.setProperty("signalStrength", savedStrength);
    MedicalLogger::instance()->writeLog("Configuration", MedicalLogger::LOG_INFO, QString("Signal strength loaded: %1").arg(savedStrength), "", "System");
    QCoreApplication::processEvents();
    QThread::msleep(300);

    // 4. 连接数据库
    updateStatus(QObject::tr("连接数据库…"));
    auto &dbMgr = DatabaseManager::instance(
        "E:/software_personal/personal_program/MyProject/MyDatabase.db"
    );
    if (!dbMgr.openDatabase()) {
        MedicalLogger::instance()->writeLog(
            "Configuration",
            MedicalLogger::LOG_ERROR,
            "Database connection failed, application exiting",
            " ",
            "System"
        );
        return -1; // 直接退出
    } else {
        // 数据库连接成功，记录成功日志
        MedicalLogger::instance()->writeLog(
            "Configuration",
            MedicalLogger::LOG_INFO,
            "Database connection established successfully",
            " ",
            "System"
        );
    }
    QCoreApplication::processEvents();
    QThread::msleep(300);

    // 5. 安装翻译器
    updateStatus(QObject::tr("加载翻译文件…"));
    {
        QTranslator translator;
        if (translator.load(":/translations/app_" + language + ".qm")) {
            a.installTranslator(&translator);
            MedicalLogger::instance()->writeLog("Translation", MedicalLogger::LOG_INFO, "Translation file loaded successfully", " ", language);
        } else {
            MedicalLogger::instance()->writeLog("Translation", MedicalLogger::LOG_ERROR, "Failed to load translation file", " ", language);
        }
    }
    QCoreApplication::processEvents();
    QThread::msleep(300);

    //6.初始化后台线程
    updateStatus(QObject::tr("初始化网络通信…"));
    // 线程
    g_receiverThread  = new QThread;
    g_processorThread = new QThread;
    // 对象（QObject 工作者）
    g_DeviceAcquisitionWorker    = new DeviceAcquisitionWorker;
    g_MeasurementDataProcessor  = new MeasurementDataProcessor;
    // 移动到线程
    g_DeviceAcquisitionWorker->moveToThread(g_receiverThread);
    g_MeasurementDataProcessor->moveToThread(g_processorThread);
    MedicalLogger::instance()->writeLog(
        "Threading",
        MedicalLogger::LOG_INFO,
        QString("DeviceAcquisitionWorker moved to thread: %1")
            .arg(reinterpret_cast<quintptr>(g_DeviceAcquisitionWorker->thread())),
        " ",
        "System"
    );
    MedicalLogger::instance()->writeLog(
        "Threading",
        MedicalLogger::LOG_INFO,
        QString("MeasurementDataProcessor moved to thread: %1")
            .arg(reinterpret_cast<quintptr>(g_MeasurementDataProcessor->thread())),
        " ",
        "System"
    );

    // 连接线程销毁信号（线程结束后自动销毁对象）
    QObject::connect(g_receiverThread, &QThread::started,
                     g_DeviceAcquisitionWorker, &DeviceAcquisitionWorker::initUdpManager);
//    QObject::connect(g_receiverThread, &QThread::started,
//                     g_DeviceAcquisitionWorker, &DeviceAcquisitionWorker::startCacheDrainer);
    QObject::connect(g_receiverThread, &QThread::finished,
                     g_DeviceAcquisitionWorker, &DeviceAcquisitionWorker::deleteLater);
    QObject::connect(g_receiverThread, &QThread::finished,
                     g_receiverThread, &QThread::deleteLater);
    QObject::connect(g_processorThread, &QThread::finished,
                     g_MeasurementDataProcessor, &MeasurementDataProcessor::deleteLater);
    QObject::connect(g_processorThread, &QThread::finished,
                     g_processorThread, &QThread::deleteLater);


    // 启动线程
    g_receiverThread->start();
    g_processorThread->start();
    MedicalLogger::instance()->writeLog("Threading", MedicalLogger::LOG_INFO, "Receiver and Processor threads started", " ", "System");
    QCoreApplication::processEvents();
    QThread::msleep(300);

    // 7. 准备主界面
    updateStatus(QObject::tr("准备主界面…"));
    QCoreApplication::processEvents();
    QThread::msleep(300);

    // 8.展示登录窗口
    globalLoginWindowPointer = new MultiUserLoginWindow();
    globalLoginWindowPointer->setAttribute(Qt::WA_AcceptTouchEvents);
    globalLoginWindowPointer->show();
    QTimer::singleShot(500, &splash, SLOT(close()));

    int ret = a.exec();
    QObject::connect(&a, &QApplication::aboutToQuit, [](){
        // 停止线程（退出事件循环并等待结束）
        if (g_receiverThread && g_receiverThread->isRunning()) {
            MedicalLogger::instance()->writeLog(
                "Threading",
                MedicalLogger::LOG_INFO,
                "Receiver thread stopping...",
                " ",
                "System"
            );
            g_receiverThread->quit();
            g_receiverThread->wait(3000); // 最多等待3秒
        }
        if (g_processorThread && g_processorThread->isRunning()) {
            MedicalLogger::instance()->writeLog(
                "Threading",
                MedicalLogger::LOG_INFO,
                "Processor thread stopping...",
                " ",
                "System"
            );
            g_processorThread->quit();
            g_processorThread->wait(3000);
        }
        delete globalLoginWindowPointer;
        globalLoginWindowPointer = nullptr;
    });
    return ret;
}
