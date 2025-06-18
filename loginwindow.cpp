#include "loginwindow.h"
#include <QPalette>
#include <QFont>
#include <QHBoxLayout>
#include <QPainter>
#include <QLinearGradient>
#include <QSettings>
#include <QApplication>
#include <QEvent>
#include <QDebug>
#include <QCloseEvent>
#include <QGraphicsDropShadowEffect>
#include "settingswidget.h"
#include "CustomMessageBox.h"
#include "toucheventhandler.h"
#include "debugmodeselector.h"

LoginWindow::LoginWindow(QWidget *parent)
    : FramelessWindow (parent)
{
    // 1) 去掉系统标题栏和边框
//    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);

    // 读取用户上次选择的语言
    QSettings settings("MyCompany", "MyApp");
    QString languageCode = settings.value("language", "zh_CN").toString();  // 默认中文
    qDebug() << "MainWindow: Loaded language:" << languageCode;

    // 设置软件语言
    changeLanguage(languageCode);

//    setWindowTitle(tr("医疗设备管理系统"));
//    setFixedSize(1024, 600);

    TouchEventHandler *touchEventHandler = new TouchEventHandler(this);  // 创建触控事件处理器
    this->installEventFilter(touchEventHandler);  // 安装事件过滤器

    // 设置深蓝背景
    QPalette pal;
    pal.setColor(QPalette::Background, QColor("#0f2243"));

    // 顶部栏部件
    QWidget *topBar = new QWidget(this);
    topBar->setFixedHeight(50);
    topBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // 设置透明背景（如果你使用渐变背景）
    topBar->setStyleSheet("background-color: transparent;");

    // 系统名称 Label（左侧）
    // 系统名称 Label（左侧）
    QLabel *iconLabel = new QLabel(this);
    QPixmap pix(":/image/icons8-tingzhen.png");
    // 缩放到合适大小，比如 24×24
    pix = pix.scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    iconLabel->setPixmap(pix);
    iconLabel->setFixedSize(pix.size());

    titleLabel = new QLabel(tr("医疗设备管理系统"), this);
    titleLabel->setStyleSheet("color: white; font-size: 18px; font-weight: bold;");
    titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    //设置按钮（右侧）
    QPushButton *btnSettings = new QPushButton(this);
    btnSettings->setIcon(QIcon(":/image/icons8-shezhi.png"));
    btnSettings->setIconSize(QSize(24, 24));
    btnSettings->setFlat(true);  // 去除按钮边框
    // 设置点击视觉反馈
    btnSettings->setStyleSheet(R"(
        QPushButton {
            border: none;
            background-color: transparent;
            border-radius: 20px; /* 让 hover/pressed 效果是圆的 */
        }
        QPushButton:pressed {
            background-color: rgba(255, 255, 255, 0.2);
        }
    )");

    connect(btnSettings, &QPushButton::clicked, this, &LoginWindow::onSettingClicked);

    clickTimer.start();  //初始化计时器，记录首次点击时间

    //顶部栏布局
    QHBoxLayout *topLayout = new QHBoxLayout(topBar);
    topLayout->addWidget(iconLabel);
    topLayout->addWidget(titleLabel);
    topLayout->addStretch();
    topLayout->addWidget(btnSettings);
    topLayout->setContentsMargins(10, 0, 10, 0);  // 左右边距


    // 创建按钮
    btnNewPerson = new QPushButton(tr("新填入物"),this);
    btnNewPerson->setIcon(QIcon(":/image/icons8-add-94.png"));
    btnNewPerson->setIconSize(QSize(30, 30));
    btnVisit = new QPushButton(tr("回\u3000\u3000访"),this);
    btnVisit->setIcon(QIcon(":/image/icons8-change-94.png"));
    btnVisit->setIconSize(QSize(30, 30));
    btnPatientList = new QPushButton(tr("患者列表"),this);
    btnPatientList->setIcon(QIcon(":/image/icons8-list-94.png"));
    btnPatientList->setIconSize(QSize(30, 30));

    btnNewPerson->setFixedHeight(50);
    btnVisit->setFixedHeight(50);
    btnPatientList->setFixedHeight(50);

    QString btnStyle = R"(
    QPushButton {
        background-color: qlineargradient(
            x1:0, y1:0, x2:0, y2:1,
            stop:0 #ffffff,
            stop:1 #e0e0e0
        );
        color: black;
        font-size: 16px;
        padding: 10px;
        border-radius: 5px;
        border: 1px solid #b0b0b0;
    }
    QPushButton:pressed {
        background-color: qlineargradient(
            x1:0, y1:0, x2:0, y2:1,
            stop:0 #cccccc,
            stop:1 #aaaaaa
        );
        padding-left: 2px;
        padding-top: 2px;
    }
    )";


    btnNewPerson->setStyleSheet(btnStyle);
    btnVisit->setStyleSheet(btnStyle);
    btnPatientList->setStyleSheet(btnStyle);

    connect(btnNewPerson, &QPushButton::clicked, this, &LoginWindow::openImplantWindow);
    connect(btnVisit, &QPushButton::clicked, this, &LoginWindow::openFollowupFormWindow);
    connect(btnPatientList, &QPushButton::clicked, this, &LoginWindow::openPatientListWidget);

    // 创建内部 widget 容器
    QWidget *buttonContainer = new QWidget(this);
    buttonContainer->setFixedSize(420, 336);  // 控制容器尺寸
    buttonContainer->setStyleSheet(R"(
    QWidget {
        background-color: qlineargradient(
            x1: 0, y1: 0, x2: 0, y2: 1,
            stop: 0 rgba(255, 255, 255, 0.08),
            stop: 1 rgba(255, 255, 255, 0.03)
        );
        border-radius: 10px;
        padding: 20px;
        border: 1px solid rgba(255, 255, 255, 0.06);
    }
    )");

    QVBoxLayout *containerLayout = new QVBoxLayout(buttonContainer);
    containerLayout->addWidget(btnNewPerson);
    containerLayout->addWidget(btnVisit);
    containerLayout->addWidget(btnPatientList);
    containerLayout->setSpacing(10);//按钮之间的间隔
    containerLayout->setContentsMargins(65, 10, 65, 10);  // 按钮与容器边距（左上右下）

    // 设置按钮容器居中
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->addWidget(topBar);
    mainLayout->addStretch();
    mainLayout->addWidget(buttonContainer, 0, Qt::AlignCenter);
    mainLayout->addStretch();

    setLayout(mainLayout);

    isInitialized = true;

    // 添加投影
    auto *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(20);
    shadow->setOffset(0, 6);
    shadow->setColor(QColor(0, 0, 0, 100));
    this->setGraphicsEffect(shadow);
}

LoginWindow::~LoginWindow() {}

void LoginWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    QLinearGradient gradient(0, 0, 0, height());

    // 设置渐变色：从深蓝到亮蓝
    gradient.setColorAt(0.0, QColor("#0f2243"));  // 顶部颜色
    gradient.setColorAt(1.0, QColor("#1a2d67"));  // 底部颜色（你可以微调）

    painter.fillRect(rect(), gradient);
}

void LoginWindow::onSettingClicked()
{
        static std::atomic<int> clickCount{0};
        static QTimer *clickTimer = nullptr;

        const int maxInterval = 2000;       // 每次点击间隔不能超过这个（最大点击窗口）
        const int decisionDelay = 500;      // 等待下次点击前的决策延迟
        static QElapsedTimer timer;

        if (!clickTimer) {
            clickTimer = new QTimer(this);
            clickTimer->setSingleShot(true);
            connect(clickTimer, &QTimer::timeout, this, [this]() {
                if (clickCount < 5) {
                    qDebug() << "打开设置界面";
                    openSettingsWindow();
                }
                clickCount = 0;  // 重置点击次数
            });
        }

        // 判断时间间隔
        if (timer.isValid() && timer.elapsed() > maxInterval) {
            clickCount = 0;  // 超过最大间隔，重置计数
        }

        clickCount++;
        timer.restart();

        if (clickCount >= 5) {
            qDebug() << "触发维护界面";
            clickTimer->stop();     // 取消打开设置界面的等待
            showHiddenWidget();     // 显示隐藏界面
            clickCount = 0;
        } else {
            clickTimer->start(decisionDelay);  // 等待是否还有下一次点击
        }

}

void LoginWindow::showHiddenWidget() {

    //添加遮罩层
    QWidget *overlay = new QWidget(this);
    overlay->setGeometry(this->rect());
    overlay->setStyleSheet("background-color: rgba(0, 0, 0, 100);"); // 可调透明度
    overlay->setAttribute(Qt::WA_TransparentForMouseEvents, false); // 拦截事件
    overlay->show();
    overlay->raise();

    //添加模糊效果
    QGraphicsBlurEffect *blur = new QGraphicsBlurEffect;
    blur->setBlurRadius(20);  // 可调强度：20~40
    this->setGraphicsEffect(blur);
    DebugModeSelector *selector = new DebugModeSelector(this);
    connect(selector,&DebugModeSelector::modeSelected,this,[=](QString mode){
        if(mode == "serial"){
            if (!maintenancewidget) {
                maintenancewidget = std::make_unique<MaintenanceWidget>();
            }
            maintenancewidget->show();
        }else if(mode == "udp"){
            if (!udpdebugwidget) {
                udpdebugwidget = std::make_unique<udpDebugWidget>();
            }
            udpdebugwidget->show();
        }
    });
    selector->exec(); //模态弹出

    // 清除遮罩和模糊
    this->setGraphicsEffect(nullptr);
    overlay->close();
    overlay->deleteLater();
}

void LoginWindow::changeLanguage(const QString &languageCode)
{
    qApp->removeTranslator(&translator);

     QString qmPath = QString(":/translations/translations/%1.qm").arg(languageCode);

    if (translator.load(qmPath)) {
        qApp->installTranslator(&translator);
        qDebug() << "语言切换成功：" << qmPath;

        //存储用户选择的语言
        QSettings settings("MyCompany", "MyApp");
        settings.setValue("language", languageCode);
    } else {
        qDebug() << "语言加载失败：" << qmPath;
        return;
    }

    // **遍历所有顶级窗口，发送 `LanguageChange` 事件**
    QEvent event(QEvent::LanguageChange);
    foreach (QWidget *widget, QApplication::topLevelWidgets()) {
        QApplication::sendEvent(widget, &event);
    }

    qDebug() << "语言切换事件已发送，所有界面应该自动更新！";
}

void LoginWindow::openSettingsWindow(){
        SettingsWidget *settingswidget = new SettingsWidget(this);       // 独立窗口
        settingswidget->setWindowFlags(Qt::Window);                         // 设置窗口
        settingswidget->setAttribute(Qt::WA_DeleteOnClose);                 // 自动释放
        settingswidget->show();                                             // 显示窗口
};

void LoginWindow::openImplantWindow() {
    if (!implantWindow) {
        implantWindow = std::make_unique<ImplantInfoWidget>();  // 只有在窗口未创建时才创建
        implantWindow->setStyleSheet(R"(
            QWidget {
                 background-color: qlineargradient(
                     x1: 0, y1: 0, x2: 0, y2: 1,
                     stop: 0 rgba(30, 50, 80, 0.9),     /* 顶部：偏亮蓝灰，透明度 0.9 */
                     stop: 1 rgba(10, 25, 50, 0.75)     /* 底部：深蓝，透明度 0.75 */
                 );
                color: white;
                font-size: 16px;
                border-radius: 10px;
            }

            QLabel {
                color: white;
                font-weight: bold;
            }

            QPushButton {
                background-color: #4a6283;
                color: white;
                padding: 3px 8px;
                border-radius: 6px;
                font-weight: bold;
                min-height: 30px;
            }

            QPushButton:pressed {
                background-color: rgba(255, 255, 255, 0.3);
            }
        )");

    }
    connect(implantWindow.get(), &ImplantInfoWidget::openSettingsWindow, this, [this]() {
        openSettingsWindow();
        qDebug() << "设置按钮已点击，打开设置界面";
    });
    connect(implantWindow.get(), &ImplantInfoWidget::implantReturnLogin, this, &LoginWindow::closeImplantWindow);

    // 添加对 destroyed 信号的监听，确保窗口销毁时清理资源
    connect(implantWindow.get(), &QObject::destroyed, this, [this]() {
        implantWindow.reset(); // 窗口被销毁后，确保指针置空
    });

    implantWindow->setWindowFlags(Qt::Window);
    implantWindow->show();
}

void LoginWindow::closeImplantWindow(){

    implantWindow->close();
}

void LoginWindow::showLoginWindow() {
    this->show(); // 显示登录界面
}

void LoginWindow::changeEvent(QEvent *event) {
    if (isInitialized && event->type() == QEvent::LanguageChange) {
        // 更新所有界面上的文本
        if (titleLabel)
        titleLabel->setText(tr("医疗设备管理系统"));
        btnNewPerson->setText(tr("新填入物"));
        btnVisit->setText(tr("回\u3000\u3000访"));
        btnPatientList->setText(tr("患者列表"));
    }
    QWidget::changeEvent(event);
}

void LoginWindow::openFollowupFormWindow() {

    if (!followupformwindow) {
    // 只有在窗口未创建时创建 FollowUpForm 窗口
    followupformwindow = std::make_unique<FollowUpForm>();
    }
    connect(followupformwindow.get(), &FollowUpForm::openSettingsWindow, this, [this]() {
        openSettingsWindow();
        qDebug() << "设置按钮已点击，打开设置界面";
    });
    connect(followupformwindow.get(), &FollowUpForm::followReturnToLogin, this, &LoginWindow::closeFollowupformwindow);

    followupformwindow->setWindowFlags(Qt::Window);
    followupformwindow->setStyleSheet(R"(
        QWidget {
             background-color: qlineargradient(
                 x1: 0, y1: 0, x2: 0, y2: 1,
                 stop: 0 rgba(30, 50, 80, 0.9),     /* 顶部：偏亮蓝灰，透明度 0.9 */
                 stop: 1 rgba(10, 25, 50, 0.75)     /* 底部：深蓝，透明度 0.75 */
             );
            color: white;
            font-size: 16px;
            border-radius: 10px;
        }

        QLabel {
            color: white;
            font-weight: bold;
        }

        QPushButton {
            background-color: #4a6283;
            color: white;
            padding: 3px 8px;
            border-radius: 6px;
            font-weight: bold;
            min-height: 30px;
        }

        QPushButton:pressed {
            background-color: rgba(255, 255, 255, 0.3);
        }
    )");


    followupformwindow->show();  // 不要将其添加到 LoginWindow 的布局中
}

void LoginWindow::closeFollowupformwindow(){
    followupformwindow->close();
}

void LoginWindow::openPatientListWidget() {
    if (!patientlistwidget) {
        patientlistwidget = std::make_unique<PatientListWidget>();  // 只有在窗口未创建时才创建
    }
    connect(patientlistwidget.get(), &PatientListWidget::openSettingsWindow, this, [this]() {
        openSettingsWindow();
        qDebug() << "设置按钮已点击，打开设置界面";
    });
    connect(patientlistwidget.get(), &PatientListWidget::patientReturnLogin, this, &LoginWindow::closePatientListWidget);

    patientlistwidget->setWindowFlags(Qt::Window);
    patientlistwidget->setStyleSheet(R"(
        QWidget {
             background-color: qlineargradient(
                 x1: 0, y1: 0, x2: 0, y2: 1,
                 stop: 0 rgba(30, 50, 80, 0.9),     /* 顶部：偏亮蓝灰，透明度 0.9 */
                 stop: 1 rgba(10, 25, 50, 0.75)     /* 底部：深蓝，透明度 0.75 */
             );
            color: white;
            font-size: 16px;
            border-radius: 10px;
        }

        QLabel {
            color: white;
            font-weight: bold;
        }

        QPushButton {
            background-color: #4a6283;
            color: white;
            padding: 3px 8px;
            border-radius: 6px;
            font-weight: bold;
            min-height: 30px;
        }

        QPushButton:pressed {
            background-color: rgba(255, 255, 255, 0.3);
        }
    )");
    patientlistwidget->show();
}

void LoginWindow::closePatientListWidget(){
    patientlistwidget->close();
}
