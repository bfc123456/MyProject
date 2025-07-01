#include <QApplication>
#include <QSplashScreen>
#include <QTimer>
#include <QScreen>
#include <QPainter>
#include <QLinearGradient>
#include <QLabel>
#include <QVBoxLayout>
#include <QFont>
#include <QProgressBar>
#include <QSettings>
#include <QTranslator>
#include <QDate>
#include "udpmanager.h"
#include "languagemanager.h"
#include "databasemanager.h"
#include "loginwindow.h"
#include "toucheventhandler.h"

LoginWindow* globalLoginWindowPointer = nullptr;

//在最顶层标记待翻译的步骤字符串
static const char* rawSteps[] = {
    QT_TRANSLATE_NOOP("App", "加载配置文件…"),
    QT_TRANSLATE_NOOP("App", "初始化网络模块…"),
    QT_TRANSLATE_NOOP("App", "连接数据库…"),
    QT_TRANSLATE_NOOP("App", "准备主界面…")
};

int main(int argc, char *argv[])
{

    QCoreApplication::setAttribute(Qt::AA_SynthesizeTouchForUnhandledMouseEvents);
    QCoreApplication::setAttribute(Qt::AA_SynthesizeMouseForUnhandledTouchEvents);

    QApplication a(argc, argv);

    TouchEventHandler *touchHandler = new TouchEventHandler();
    a.installEventFilter(touchHandler);

    UdpManager udpManager;
    if (!udpManager.startListening(5005)) {
        qDebug() << "UDP监听失败";
        return -1;
    }

    LanguageManager::instance();

    //只初始化一次，传入路径
    DatabaseManager::instance("E:/software_personal/personal_program/MyProject/MyDatabase.db");
    if (!DatabaseManager::instance("E:/software_personal/personal_program/MyProject/MyDatabase.db").openDatabase()) {
        return -1;
    }

    QSettings settings("MyCompany", "MyApp");
    QString language = settings.value("language", "zh_CN").toString();

    QTranslator translator;
    if (translator.load(":/translations/app_" + language + ".qm")) {
        a.installTranslator(&translator);
    }

    QFont font("Microsoft YaHei", 12);
    a.setFont(font);

    QScreen *screen = QGuiApplication::primaryScreen();
    QSize screenSize = screen->availableGeometry().size();

    QPixmap pixmap(screenSize);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    QLinearGradient gradient(0, 0, 0, pixmap.height());
    gradient.setColorAt(0.0, QColor("#102b4e"));
    gradient.setColorAt(1.0, QColor("#1c3f5f"));
    painter.fillRect(pixmap.rect(), gradient);
    painter.end();

    QSplashScreen splash(pixmap);
    splash.setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    splash.setAttribute(Qt::WA_TranslucentBackground);
    splash.setFixedSize(screenSize);
    splash.showFullScreen();

    int logoW = screenSize.width() * 0.20;
    QLabel *logoLabel = new QLabel(&splash);
    logoLabel->setPixmap(QPixmap(":/image/logoimage.jpg").scaledToWidth(logoW, Qt::SmoothTransformation));
    logoLabel->setAlignment(Qt::AlignCenter);
    logoLabel->setStyleSheet("background-color: rgba(255,255,255,0.05); border-radius: 12px; padding: 10px;");

    QLabel *statusLabel = new QLabel(QObject::tr("系统初始化中…"), &splash);
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setStyleSheet("font-size: 22px; color: white; font-weight: 600;");

    QLabel *versionLabel = new QLabel(QObject::tr("Version 1.0.0"), &splash);
    versionLabel->setAlignment(Qt::AlignCenter);
    versionLabel->setStyleSheet("font-size:" + QString::number(screenSize.height()*0.03) + "px; color:rgba(255,255,255,0.6);");

    int currentYear = QDate::currentDate().year();
    QLabel *copyrightLabel = new QLabel(QObject::tr("© %1 芯联心 SI-Linking").arg(currentYear), &splash);
    copyrightLabel->setAlignment(Qt::AlignCenter);
    copyrightLabel->setStyleSheet("font-size:" + QString::number(screenSize.height()*0.013) + "px; color:rgba(255,255,255,0.4);");

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

    //--- 把 rawSteps 翻译成当前语言的 QStringList ---
    QStringList steps;
    const int stepCount = sizeof(rawSteps) / sizeof(rawSteps[0]);
    for (int i = 0; i < stepCount; ++i) {
        steps << QCoreApplication::translate("App", rawSteps[i]);
    }

    // --- 用定时器依次更新 statusLabel ---
    int stepIndex = 0;
    QTimer *timer = new QTimer(&a);
    QObject::connect(timer, &QTimer::timeout, [&]() {
        if (stepIndex < steps.size()) {
            statusLabel->setText(steps[stepIndex]);
            ++stepIndex;
        } else {
            timer->stop();
            splash.close();
            globalLoginWindowPointer->show();
        }
    });
    timer->start(600);

    globalLoginWindowPointer = new LoginWindow();
    globalLoginWindowPointer->setFixedSize(1024, 600);
    globalLoginWindowPointer->setAttribute(Qt::WA_AcceptTouchEvents);

    int result = a.exec();

    delete globalLoginWindowPointer;
    globalLoginWindowPointer = nullptr;
    return result;
}
