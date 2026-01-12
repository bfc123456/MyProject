#include <QApplication>
#include <QScreen>
#include <QPainter>
#include <QFont>
#include <QSettings>
#include <QTranslator>
#include "medicallogger.h"
#include "udpmanager.h"
#include "languagemanager.h"
#include "databasemanager.h"
#include "multiuserloginwindow.h"
#include "toucheventhandler.h"
#include "global.h"
#include "deviceacquisitionworker.h"
#include "measurementdataprocessor.h"
#include "measurementdata.h"
#include "updatemanager.h"
#include "splashscreen.h"
#include <QQmlApplicationEngine>

int main(int argc, char *argv[])
{
    //触控支持
    QCoreApplication::setAttribute(Qt::AA_SynthesizeTouchForUnhandledMouseEvents);
    QCoreApplication::setAttribute(Qt::AA_SynthesizeMouseForUnhandledTouchEvents);
    qputenv("QT_IM_MODULE", "qtvirtualkeyboard");          // 使用 Qt 虚拟键盘

    QApplication a(argc, argv);

    QQmlApplicationEngine engine;
       const QUrl url(QStringLiteral("qrc:/qml/main.qml"));
       QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                        &a, [url](QObject *obj, const QUrl &objUrl) {
           if (!obj && url == objUrl)
               QCoreApplication::exit(-1);
       }, Qt::QueuedConnection);
       engine.load(url);

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


     // 初始化过程
    splash.updateStatus(QObject::tr("加载本地配置…"), 10);
    QSettings settings("MyCompany", "MyApp");
    QString language = settings.value("language", "zh_CN").toString();
    QCoreApplication::processEvents();
    QThread::msleep(300);

    // 检查更新
    splash.updateStatus(QObject::tr("检查更新…"), 30);
    UpdateManager um;
    QString updateMsg;
    um.applyPendingIfAny(&updateMsg);
    //想在启动后给出提示    if (!updateMsg.isEmpty()) {
        splash.updateStatus(QObject::tr("检测到软件更新: %1").arg(updateMsg), 30);
    } else {
        splash.updateStatus(QObject::tr("未检测到软件更新"), 30);
    }

    // 读取信号强度（默认 70）
    splash.updateStatus(QObject::tr("读取信号强度…"), 50);
    int savedStrength = settings.value("system/signalStrength", 70).toInt();
    a.setProperty("signalStrength", savedStrength);
    QCoreApplication::processEvents();
    QThread::msleep(300);

    //连接数据库
    splash.updateStatus(QObject::tr("连接数据库…"), 60);
    auto &dbMgr = DatabaseManager::instance(
        "E:/software_personal/personal_program/MyProject/MyDatabase.db"
    );
    if (!dbMgr.openDatabase()) {
        return -1; // 直接退出
    }
    QCoreApplication::processEvents();
    QThread::msleep(300);

    // 5. 安装翻译器
    splash.updateStatus(QObject::tr("加载翻译文件…"), 70);
    QTranslator translator;
    if (translator.load(":/translations/app_" + language + ".qm")) {
        a.installTranslator(&translator);
    }
    QCoreApplication::processEvents();
    QThread::msleep(300);

    //初始化后台线程
    splash.updateStatus(QObject::tr("初始化网络通信…"), 80);
    // 线程
    g_receiverThread  = new QThread;
    g_processorThread = new QThread;
    // 对象（QObject 工作者）
    g_DeviceAcquisitionWorker    = new DeviceAcquisitionWorker;
    g_MeasurementDataProcessor  = new MeasurementDataProcessor;
    // 移动到线程
    g_DeviceAcquisitionWorker->moveToThread(g_receiverThread);
    g_MeasurementDataProcessor->moveToThread(g_processorThread);

    // 连接线程销毁信号（线程结束后自动销毁对象）
    QObject::connect(g_receiverThread, &QThread::started,
                     g_DeviceAcquisitionWorker, &DeviceAcquisitionWorker::initUdpManager);
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
    QCoreApplication::processEvents();
    QThread::msleep(300);

    // 7. 准备主界面
    splash.updateStatus(QObject::tr("准备主界面…"), 90);

    // 8.展示登录窗口
    globalLoginWindowPointer = new MultiUserLoginWindow();
    globalLoginWindowPointer->setAttribute(Qt::WA_AcceptTouchEvents);
    globalLoginWindowPointer->show();
    QTimer::singleShot(500, &splash, SLOT(close()));

    int ret = a.exec();
    QObject::connect(&a, &QApplication::aboutToQuit, [](){
        // 停止线程（退出事件循环并等待结束）
        if (g_receiverThread && g_receiverThread->isRunning()) {
            g_receiverThread->quit();
            g_receiverThread->wait(3000); // 最多等待3秒
        }
        if (g_processorThread && g_processorThread->isRunning()) {
            g_processorThread->quit();
            g_processorThread->wait(3000);
        }
        delete globalLoginWindowPointer;
        globalLoginWindowPointer = nullptr;
    });
    return ret;
}
