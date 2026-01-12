//1) Project Headers
#include "medicallogger.h"
#include "udpmanager.h"
#include "languagemanager.h"
#include "databasemanager.h"
#include "toucheventhandler.h"
#include "global.h"
#include "deviceacquisitionworker.h"
#include "measurementdataprocessor.h"
#include "measurementdata.h"
#include "updatemanager.h"
#include "splashscreen.h"

//2) Qt Headers:
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
#include <QQmlApplicationEngine>

// ---- 顶部：声明一个统一收尾函数 ----
static void GracefulShutdown() {
    // (A) 先让 worker 在各自线程里停止（真正关闭 socket/定时器/断信号）
    if (g_pDeviceAcquisitionWorkerPtr)
        QMetaObject::invokeMethod(g_pDeviceAcquisitionWorkerPtr, "stop",
                                  Qt::BlockingQueuedConnection);
    if (g_pMeasurementDataProcessorPtr)
        QMetaObject::invokeMethod(g_pMeasurementDataProcessorPtr, "stop",
                                  Qt::BlockingQueuedConnection);

    // (B) 再停线程并等待
    if (g_pProcessorThreadPtr)  { g_pProcessorThreadPtr->quit();  g_pProcessorThreadPtr->wait(3000); }
    if (g_pReceiverThreadPtr) { g_pReceiverThreadPtr->quit(); g_pReceiverThreadPtr->wait(3000); }

    // (C) 释放 UI（断信号+清排队事件，防晚到事件打到已删对象）
    if (g_pGlobalImplantMonitorPtr) {
        QObject::disconnect(nullptr, nullptr, g_pGlobalImplantMonitorPtr, nullptr);
        QCoreApplication::removePostedEvents(g_pGlobalImplantMonitorPtr, 0);
        delete g_pGlobalImplantMonitorPtr;
        g_pGlobalImplantMonitorPtr = nullptr;
    }

    // (D) 日志系统在退出期关闭且“静音”
    qInstallMessageHandler(nullptr);                  // 恢复默认消息处理器
    MedicalLogger::instance()->shutdown();            // 刷盘并关文件
}

int main(int argc, char *argv[])
{
    //触控支持
    QCoreApplication::setAttribute(Qt::AA_SynthesizeTouchForUnhandledMouseEvents);
    QCoreApplication::setAttribute(Qt::AA_SynthesizeMouseForUnhandledTouchEvents);

    qputenv("QT_IM_MODULE", QByteArray("qtvirtualkeyboard"));

    QApplication a(argc, argv);
    LanguageManager::instance();

    QQmlApplicationEngine engine;

    // 打印所有 QML 导入路径
    qDebug() << "QML 导入路径:" << engine.importPathList();

    engine.load(QUrl(QStringLiteral("qrc:/qml/KeyboardOverlay.qml")));

    const QUrl url(QStringLiteral("qrc:/qml/KeyboardOverlay.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &a, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    qputenv("QT_LOGGING_RULES", "qt.virtualkeyboard.styles=true;qt.qml.imports=true");

    a.installEventFilter(new TouchEventHandler());
    a.setApplicationName("MeasurementtApp");
    qRegisterMetaType<MeasurementData>("MeasurementData");

    MedicalLogger::instance()->init();   //启动日志记录
    qInstallMessageHandler(MedicalLogger::qtMessageHandler);  // 设置自定义的 Qt 消息处理器

    //当应用退出时，清理日志文件
    QObject::connect(&a,&QCoreApplication::aboutToQuit,[](){
       //清理日志资源
        MedicalLogger::instance()->shutdown();
    });

    //Splash 界面搭建
    // 获取主屏幕并初始化 SplashScreen
    QScreen *screen = QGuiApplication::primaryScreen();
    SplashScreen splash(screen);
    // 显示启动界面
    splash.show();


    // 1. 加载本地设置
    splash.UpdateStatus(QObject::tr("加载本地配置…"),30);
    QSettings settings("MyCompany", "MyApp");
    // 关键：通过 LanguageManager 获取上次保存的语言
    QString language = LanguageManager::instance().currentLanguage();
    QCoreApplication::processEvents();
    QThread::msleep(300);

    // 检查更新
    splash.UpdateStatus(QObject::tr("检查更新…"), 30);
    UpdateManager um;
    QString updateMsg;
    um.applyPendingIfAny(&updateMsg);
    //想在启动后给出提示
    if (!updateMsg.isEmpty()) {
        splash.UpdateStatus(QObject::tr("检测到软件更新: %1").arg(updateMsg), 30);
    } else {
        splash.UpdateStatus(QObject::tr("未检测到软件更新"), 30);
    }

    // 读取信号强度（默认 70）
    splash.UpdateStatus(QObject::tr("读取信号强度…"), 50);
    int savedStrength = settings.value("system/signalStrength", 70).toInt();
    a.setProperty("signalStrength", savedStrength);
    QCoreApplication::processEvents();
    QThread::msleep(300);

    //连接数据库
    splash.UpdateStatus(QObject::tr("连接数据库…"), 60);
    auto &dbMgr = DatabaseManager::instance(
        "E:/software_personal/personal_program/MyProject/MyDatabase.db"
    );
    if (!dbMgr.openDatabase()) {
        return -1; // 直接退出
    }
    QCoreApplication::processEvents();
    QThread::msleep(300);

    // 5. 安装翻译器
    splash.UpdateStatus(QObject::tr("加载翻译文件…"), 70);
    QTranslator translator;
    if (translator.load(":/translations/app_" + language + ".qm")) {
        a.installTranslator(&translator);
    }
    QCoreApplication::processEvents();
    QThread::msleep(300);

    //初始化后台线程
    splash.UpdateStatus(QObject::tr("初始化网络通信…"), 80);
    // 线程
    g_pReceiverThreadPtr  = new QThread;
    g_pProcessorThreadPtr = new QThread;
    // 对象（QObject 工作者）
    g_pDeviceAcquisitionWorkerPtr    = new DeviceAcquisitionWorker;
    g_pMeasurementDataProcessorPtr  = new MeasurementDataProcessor;
    // 移动到线程
    g_pDeviceAcquisitionWorkerPtr->moveToThread(g_pReceiverThreadPtr);
    g_pMeasurementDataProcessorPtr->moveToThread(g_pProcessorThreadPtr);

    // 连接线程销毁信号（线程结束后自动销毁对象）
    QObject::connect(g_pReceiverThreadPtr, &QThread::started,
                     g_pDeviceAcquisitionWorkerPtr, &DeviceAcquisitionWorker::initUdpManager);
    QObject::connect(g_pReceiverThreadPtr, &QThread::finished,
                     g_pDeviceAcquisitionWorkerPtr, &DeviceAcquisitionWorker::deleteLater);
    QObject::connect(g_pReceiverThreadPtr, &QThread::finished,
                     g_pReceiverThreadPtr, &QThread::deleteLater);
    QObject::connect(g_pProcessorThreadPtr, &QThread::finished,
                     g_pMeasurementDataProcessorPtr, &MeasurementDataProcessor::deleteLater);
    QObject::connect(g_pProcessorThreadPtr, &QThread::finished,
                     g_pProcessorThreadPtr, &QThread::deleteLater);


    // 启动线程
    g_pReceiverThreadPtr->start();
    g_pProcessorThreadPtr->start();
    QCoreApplication::processEvents();
    QThread::msleep(300);

    // 7. 准备主界面
    splash.UpdateStatus(QObject::tr("准备主界面…"), 90);

    // 8.展示登录窗口
    g_pGlobalImplantMonitorPtr = new ImplantMonitor();
    g_pGlobalImplantMonitorPtr->setAttribute(Qt::WA_AcceptTouchEvents);
    g_pGlobalImplantMonitorPtr->show();
    QTimer::singleShot(500, &splash, SLOT(close()));

    int ret = a.exec();
    GracefulShutdown();
    return ret;
}
