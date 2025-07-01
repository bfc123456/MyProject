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
#include "CustomMessageBox.h"
#include "toucheventhandler.h"
#include "debugmodeselector.h"
#include <QGuiApplication>
#include <QScreen>


LoginWindow::LoginWindow(QWidget *parent)
    : FramelessWindow (parent)
{
    // 获取屏幕分辨率
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int screenWidth = screenGeometry.width();
    int screenHeight = screenGeometry.height();

    // 计算缩放比例
    float scaleX = (float)screenWidth / 1024;
    float scaleY = (float)screenHeight / 600;

    // 设置窗口初始大小
    this->resize(1024 * scaleX, 600 * scaleY);  // 设置为基于目标分辨率的大小

    // 读取用户上次选择的语言
    QSettings settings("MyCompany", "MyApp");
    QString languageCode = settings.value("language", "zh_CN").toString();  // 默认中文
    qDebug() << "MainWindow: Loaded language:" << languageCode;

    // 设置软件语言
    changeLanguage(languageCode);

    TouchEventHandler *touchEventHandler = new TouchEventHandler(this);  // 创建触控事件处理器
    this->installEventFilter(touchEventHandler);  // 安装事件过滤器

    // 设置深蓝背景
    QPalette pal;
    pal.setColor(QPalette::Background, QColor("#0f2243"));

    // 顶部栏部件
    QWidget *topBar = new QWidget(this);
    topBar->setFixedHeight(50 * scaleY);
    topBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // 设置透明背景
    topBar->setStyleSheet("background-color: transparent;");

    // 系统名称 Label（左侧）
    QLabel *iconLabel = new QLabel(this);
    QPixmap pix(":/image/icons8-tingzhen.png");
    // 缩放到合适大小，比如 24×24
    pix = pix.scaled(24 * scaleX, 24 * scaleY, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    iconLabel->setPixmap(pix);
    iconLabel->setFixedSize(pix.size());

    titleLabel = new QLabel(tr("医疗设备管理系统"), this);
    titleLabel->setStyleSheet("color: white; font-size: 25px; font-weight: bold;");
    titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    //设置按钮（右侧）
    QPushButton *btnSettings = new QPushButton(this);
    btnSettings->setIcon(QIcon(":/image/icons8-shezhi.png"));
    btnSettings->setIconSize(QSize(30 * scaleX, 30 * scaleY));
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
    topLayout->setContentsMargins(10 * scaleX, 0 * scaleY, 10 * scaleX, 0 * scaleY);  // 左右边距


    // 创建登录控件

    usernameCombox = new CustomComboBox(50 * scaleY, this);
    usernameCombox->setEditable(true);
    usernameCombox->lineEdit()->setAlignment(Qt::AlignCenter);
    usernameCombox->lineEdit()->setReadOnly(true);  // 保持只读，不影响选择
    usernameCombox->setObjectName("usernameCombox");
    passwordEdit = new QLineEdit(this);
    passwordEdit->setMinimumHeight(50 * scaleY);
    passwordEdit->setObjectName("passwordEdit");
    loginButton = new QPushButton(tr("登录"), this);
    loginButton->setMinimumHeight(50 * scaleY);
    loginButton->setStyleSheet(R"(
        QPushButton {
            background-color: #0078D7;
            color: white;
            border-radius: 8px;
            padding: 8px 15px;           /* 上下16，左右30，增大触控区 */
            font-weight: 600;             /* 更粗，但不至于溢出 */
            font-size: 20px;              /* 更大字号，适合触控屏 */
        }
        QPushButton:pressed {
            background-color: #005A9E;
        }
    )");
    connect(loginButton, &QPushButton::clicked, this, &LoginWindow::onLoginClicked);
    connect(passwordEdit, &QLineEdit::textChanged, this, &LoginWindow::clearErrorMessage);  // 用户修改密码时清除错误

    usernameCombox->addItem(tr("家用模式"));
    usernameCombox->addItem(tr("植入模式"));

    usernameCombox->setStyleSheet(R"(
        QComboBox {
            font-size: 26px;
            color: white;
            padding: 10px;
            background-color: rgba(255,255,255,0.05);
            border: 1px solid rgba(255,255,255,0.2);
            border-radius: 10px;
        }

        QComboBox QAbstractItemView {
            background-color: rgba(30, 40, 60, 0.9);
            border: 1px solid rgba(255,255,255,0.2);
            border-radius: 8px;
            selection-background-color: rgba(0, 120, 215, 0.4);
            selection-color: white;
            outline: none;
        }

        QAbstractItemView::item {
            height: 20px;
            padding-left: 12px;
            color: white;
            background-color: transparent;
        }

        QComboBox::drop-down {
            subcontrol-origin: padding;
            subcontrol-position: top right;
            width: 45px;
            border-left: 1px solid rgba(255,255,255,0.1);
            background-color: transparent;
        }

        QComboBox::down-arrow {
            image: url(:/image/icons-pulldown.png);
            width: 24px;
            height: 24px;
        }
    )");

    usernameCombox->setMinimumHeight(50 * scaleY);
    passwordEdit->setPlaceholderText(tr("请输入六位密码"));
    passwordEdit->setAlignment(Qt::AlignCenter);
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setStyleSheet(R"(
        QLineEdit {
            color: white;  /* 仅修改文字颜色为白色 */
        }
    )");

    //接入虚拟键盘
    currentKeyboard = CustomKeyboard::instance(this);
    currentKeyboard->registerEdit(passwordEdit,QPoint(-120*scaleX,120*scaleY));

    // 初始化错误信息标签（默认不可见）
    errorLabel = new QLabel(this);
    errorLabel->setFixedHeight(15 * scaleY);
    errorLabel->setStyleSheet(QString(
        "color: rgba(200, 200, 200, 1);"
        "background-color: transparent;"
        "border: none;"
        "font-size: %1px;"
    ).arg(16 * scaleY));
    errorLabel->setText(" ");
    m_lastError = NoError;

    passwordEdit->setStyleSheet(R"(
        QLineEdit {
            font-size: 26px;         /* 放大字体 */
            color: white;
            padding: 10px;
            background-color: rgba(255,255,255,0.05);
            border: 1px solid rgba(255,255,255,0.2);
            border-radius: 10px;
        }
    )");

    // 创建内部 widget 容器
    QWidget *buttonContainer = new QWidget(this);
    buttonContainer->setFixedSize(450 * scaleX, 300 * scaleY);  // 控制容器尺寸
    buttonContainer->setStyleSheet(R"(
    QWidget {
        background-color: qlineargradient(
            x1: 0, y1: 0, x2: 0, y2: 1,
            stop: 0 rgba(255, 255, 255, 0.08),
            stop: 1 rgba(255, 255, 255, 0.03)
        );
        border-radius: 10px;
        padding: 2px;
        border: 1px solid rgba(255, 255, 255, 0.06);
    }
    )");

    QVBoxLayout *containerLayout = new QVBoxLayout(buttonContainer);
    containerLayout->addWidget(usernameCombox);
    containerLayout->setSpacing(20 * scaleY);
    containerLayout->addWidget(passwordEdit);
    containerLayout->addWidget(errorLabel);
    containerLayout->addWidget(loginButton);
    containerLayout->setContentsMargins(50 * scaleX, 35 * scaleY, 50 * scaleX, 35 * scaleY);  // 按钮与容器边距（左上右下）

    // 设置按钮容器居中
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->addWidget(topBar);
    mainLayout->addStretch();
    mainLayout->addWidget(buttonContainer, 0, Qt::AlignCenter);
    mainLayout->addStretch();

    setLayout(mainLayout);

    isInitialized = true;

}

LoginWindow::~LoginWindow() {}

void LoginWindow::onLoginClicked(){

        // 每次点击先重置
        m_lastError = NoError;
        errorLabel->clear();

       // 获取用户输入
       QString selectedRole = usernameCombox->currentText(); // 获取角色（医生或患者）
       QString password = passwordEdit->text(); // 获取密码

       // 检查密码的长度是否为6位
       if (password.length() != 6)
       {
           m_lastError = ErrLength;
           errorLabel->setText(tr("密码必须是六位"));  // 设置错误信息
           return;
       }

       // 根据选择的角色验证密码
       if (selectedRole == tr("家用模式") && password == tr("123456")) // 假设患者的密码是123456
       {
           // 打开患者界面
           openFollowupFormWindow();
       }
       else if (selectedRole == tr("植入模式") && password == tr("000000")) // 假设医生的密码是000000
       {
           // 打开医生界面
           openImplantWindow();
       }
       else
       {
           m_lastError = ErrAuth;
           // 密码错误
           errorLabel->setText(tr("用户或密码错误"));  // 设置错误信息
       }
}

void LoginWindow::clearErrorMessage()
{
    m_lastError = NoError;
    errorLabel->clear();  // 清除错误信息
}

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
        settingswidget = new SettingsWidget(this);       // 独立窗口
        settingswidget->setWindowFlags(Qt::Window);                         // 设置窗口
        settingswidget->setAttribute(Qt::WA_DeleteOnClose);                 // 自动释放
        settingswidget->show();                                             // 显示窗口
        connect(settingswidget,&SettingsWidget::requestDelete,this,[=](){
            settingswidget->deleteLater();  // 安全删除
            settingswidget = nullptr;
        });
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
    this->hide();
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
        loginButton->setText(tr("登录"));

        usernameCombox->clear();
        usernameCombox->addItem(tr("家用模式"));
        usernameCombox->addItem(tr("植入模式"));

        //错误标签
        //根据上次的错误状态重新设置一次文本
        switch (m_lastError) {
        case ErrLength:
            errorLabel->setText(tr("密码必须是六位"));
            break;
        case ErrAuth:
            errorLabel->setText(tr("用户或密码错误"));
            break;
        case NoError:
        default:
            errorLabel->setText(" ");
            break;
        }

    }
    QWidget::changeEvent(event);
}

void LoginWindow::openFollowupFormWindow() {

    if (!followupformwindow) {
    // 只有在窗口未创建时创建 FollowUpForm 窗口
    followupformwindow = std::make_unique<FollowUpForm>();
    }
//    connect(followupformwindow.get(), &FollowUpForm::openSettingsWindow, this, [this]() {
//        openSettingsWindow();
//        qDebug() << "设置按钮已点击，打开设置界面";
//    });
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

void LoginWindow::closeFollowupformwindow() {
        qDebug() << "Closing followup form window...";
    if (followupformwindow) {
         qDebug() << "Closing and deleting followup form window...";
        followupformwindow->close();        // 关闭窗口
        followupformwindow->deleteLater();  // 延迟释放内存（安全）
        followupformwindow = nullptr;       // 避免悬挂指针
    }
}
