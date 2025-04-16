#include "implantinfowidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QDebug>
#include <QDialog>
#include <QGraphicsBlurEffect>
#include <QEvent>

ImplantInfoWidget::ImplantInfoWidget(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle(tr("新植入物管理界面"));
    setFixedSize(1024, 600);
    this->setObjectName("ImplantinfoWidget");
    this->setStyleSheet(R"(
    QWidget#ImplantinfoWidget {
        background-color: qlineargradient(
            x1: 0, y1: 1,
            x2: 1, y2: 0,
            stop: 0 rgba(6, 15, 30, 255),      /* 更暗靛蓝：左下 */
            stop: 0.5 rgba(18, 35, 65, 255),   /* 中段冷蓝 */
            stop: 1 rgba(30, 60, 100, 255)     /* 右上：深蓝灰 */
        );
    }
    )");

    // 顶部栏
    QWidget *topBar = new QWidget(this);
    topBar->setObjectName("TopBar");
    topBar->setStyleSheet(R"(
        #TopBar {
            background-color: qlineargradient(
                x1: 0, y1: 0, x2: 0, y2: 1,
                stop: 0 rgba(25, 50, 75, 0.9),
                stop: 1 rgba(10, 20, 30, 0.85)
            );
            border-radius: 10px;
            border: 1px solid rgba(255, 255, 255, 0.08); /* 边缘高光 */
        }
    )");

    topBar->setFixedHeight(50);


    QLabel *titleLabel = new QLabel(tr("新植入物"), this);
    titleLabel->setFixedSize(160, 35);
    titleLabel->setAutoFillBackground(false);                   // 不自动填充背景
    titleLabel->setAttribute(Qt::WA_TranslucentBackground);     // 启用透明背景
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: white;");
    titleLabel->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);

    QPushButton *btnSettings = new QPushButton(this);
    connect(btnSettings, &QPushButton::clicked, this, &ImplantInfoWidget::openSettingsWindow);  // 连接点击事件到槽函数
    btnSettings->setIcon(QIcon(":/image/icons8-shezhi.png"));
    btnSettings->setIconSize(QSize(24, 24));
    btnSettings->setFlat(true);
    btnSettings->setStyleSheet(R"(
                               QPushButton {
                               border: none;
                               background-color: transparent;
                               border-radius: 20px;
                               }
                               QPushButton:pressed {
                               background-color: rgba(255, 255, 255, 0.2);
                               }
                               )");
    QHBoxLayout *topLayout = new QHBoxLayout(topBar);
    topLayout->addWidget(titleLabel);
    topLayout->addStretch();
    topLayout->addWidget(btnSettings);
    topLayout->setContentsMargins(10, 0, 10, 0);

    // 表单部分
    QLabel *serialLabel = new QLabel(tr("传感器序列号"));
    serialLabel->setFixedSize(180, 40);
    serialLabel->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    serialInput = new QLineEdit();
    serialInput->setFixedSize(400, 50);
    serialInput->setPlaceholderText(tr("请输入传感器序列号"));

    QLabel *checksumLabel = new QLabel(tr("校准码"));
    checksumLabel->setFixedSize(180, 40);
    checksumLabel->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    checksumInput = new QLineEdit();
    checksumInput->setFixedSize(400, 50);
    checksumInput->setPlaceholderText(tr("请输入校准码"));

    QLabel *implantDoctorLabel = new QLabel(tr("植入医生"));
    implantDoctorLabel->setFixedSize(180, 40);
    implantDoctorLabel->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    implantDoctorInput = new QLineEdit();
    implantDoctorInput->setFixedSize(400, 50);
    implantDoctorInput->setPlaceholderText(tr("请输入植入医生姓名"));

    QLabel *treatDoctorLabel = new QLabel(tr("治疗医生"));
    treatDoctorLabel->setFixedSize(180, 40);
    treatDoctorLabel->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    treatDoctorInput = new QLineEdit();
    treatDoctorInput->setFixedSize(400, 50);
    treatDoctorInput->setPlaceholderText(tr("请输入治疗医生姓名"));

    QLabel *dateLabel = new QLabel(tr("植入日期"));
    dateLabel->setFixedSize(180, 40);
    dateLabel->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    implantDateInput = new QDateEdit(this);
    implantDateInput->setCalendarPopup(true);
    implantDateInput->setDate(QDate(2025, 1, 1));    // 设置默认日期为2025年1月1日

    implantDateInput->setFixedSize(400, 50);
    implantDateInput->setCalendarPopup(true);



//    //将事件过滤器安装到主线程上
//    QMetaObject::invokeMethod(this, [this]() {
//        if (!eventFilterInstalled) {
//        this->serialInput->installEventFilter(CustomKeyboard::instance(this));
//        this->checksumInput->installEventFilter(CustomKeyboard::instance(this));
//        this->treatDoctorInput->installEventFilter(CustomKeyboard::instance(this));
//        this->implantDoctorInput->installEventFilter(CustomKeyboard::instance(this));
//        eventFilterInstalled = true;
//        }
//    });

    QGridLayout *formLayout = new QGridLayout();
    formLayout->addWidget(serialLabel, 0, 0);
    formLayout->addWidget(serialInput, 1, 0);
    formLayout->addWidget(implantDoctorLabel, 0, 1);
    formLayout->addWidget(implantDoctorInput, 1, 1);
    formLayout->addWidget(checksumLabel, 2, 0);
    formLayout->addWidget(checksumInput, 3, 0);
    formLayout->addWidget(treatDoctorLabel, 2, 1);
    formLayout->addWidget(treatDoctorInput, 3, 1);
    formLayout->addWidget(dateLabel, 4, 0);
    formLayout->addWidget(implantDateInput, 5, 0);

    QWidget *formWidget = new QWidget();
    formWidget->setFixedHeight(350);
    formWidget->setStyleSheet(R"(
    QWidget {
        background-color: qlineargradient(
            x1: 0, y1: 0, x2: 0, y2: 1,
            stop: 0 rgba(25, 50, 75, 0.9),
            stop: 1 rgba(10, 20, 30, 0.85)
        );
        border-radius: 10px;
        border: 1px solid rgba(255, 255, 255, 0.08); /* 边缘高光 */
    }

    QLabel {
        color: white;
        font-weight: bold;
        font-size: 14px;
    }

    /* 输入控件样式 */
    QLineEdit, QDateEdit {
        background-color: qlineargradient(
            x1: 0, y1: 0, x2: 0, y2: 1,
            stop: 0 #32495d,
            stop: 1 #1f2e3b
        );
        border: 1px solid #3e5261;
        border-radius: 4px;
        padding: 6px;
        color: white;
        font-size: 14px;
    }

    /* 选中状态高亮 */
    QLineEdit:focus, QDateEdit:focus {
        border: 1px solid #66bbee;
        background-color: #2c3e4f;
    }

    )");



    formWidget->setLayout(formLayout);

    // 按钮部分
    backButton = new QPushButton(tr("返回"));
    QObject::connect(backButton, &QPushButton::clicked, this,&ImplantInfoWidget::implantReturnLogin);

    backButton->setIcon(QIcon(":/image/icons8-return.png"));

    backButton->setStyleSheet(R"(
    QPushButton {
        background-color: qlineargradient(
            x1:0, y1:0, x2:0, y2:1,
            stop:0 rgba(95, 169, 246, 180),
            stop:1 rgba(49, 122, 198, 180)
        );
        border: 1px solid rgba(163, 211, 255, 0.6); /* 半透明高光边框 */
        border-radius: 6px;
        color: white;
        font-weight: bold;
        font-size: 14px;
        padding: 8px 20px;
    }

    QPushButton:pressed {
        background-color: qlineargradient(
            stop: 0 rgba(47, 106, 158, 200),
            stop: 1 rgba(31, 78, 121, 200)
        );
        padding-left: 2px;
        padding-top: 2px;
    }
    )");

    continueButton = new QPushButton(tr("继续"));
    connect(continueButton, &QPushButton::clicked, this, &ImplantInfoWidget::showImplantationSiteWidget);
    continueButton->setIcon(QIcon(":/image/icons8-next.png"));

    continueButton->setStyleSheet(R"(
    QPushButton {
        background-color: qlineargradient(
            stop: 0 rgba(110, 220, 145, 180),
            stop: 1 rgba(58, 170, 94, 180)
        );
        border: 1px solid rgba(168, 234, 195, 0.6);
        border-radius: 6px;
        color: white;
        font-weight: bold;
        font-size: 14px;
        padding: 8px 20px;
    }

    QPushButton:pressed {
        background-color: qlineargradient(
            stop: 0 rgba(44, 128, 73, 200),
            stop: 1 rgba(29, 102, 53, 200)
        );
        padding-left: 2px;
        padding-top: 2px;
    }
    )");

    backButton->setFixedSize(120, 40);
    continueButton->setFixedSize(120, 40);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setContentsMargins(20, 10, 20, 10);
    buttonLayout->addWidget(backButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(continueButton);

    QWidget *buttonWidget = new QWidget();
    buttonWidget->setFixedHeight(60);

    buttonWidget->setStyleSheet(R"(
    /* 外层容器，带渐变 + 边缘高光 */
    QWidget {
        background-color: qlineargradient(
            x1: 0, y1: 0, x2: 0, y2: 1,
            stop: 0 rgba(25, 50, 75, 0.9),
            stop: 1 rgba(10, 20, 30, 0.85)
        );
        border-radius: 10px;
        border: 1px solid rgba(255, 255, 255, 0.08); /* 边缘高光 */
    }
    )");



    buttonWidget->setLayout(buttonLayout);

    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(80, 15, 80, 15);
    mainLayout->addWidget(topBar);
    mainLayout->addWidget(formWidget);
    mainLayout->addSpacing(30);
    mainLayout->addWidget(buttonWidget);
    mainLayout->addSpacing(30);
    setLayout(mainLayout);
}

ImplantInfoWidget::~ImplantInfoWidget() {

}

void ImplantInfoWidget::showImplantationSiteWidget()
{
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

    //创建提示弹窗
    QDialog prompt(this);
    prompt.setStyleSheet(R"(
        QDialog {
            background-color: qlineargradient(
                x1: 0, y1: 0, x2: 0, y2: 1,
                stop: 0 rgba(15, 34, 67, 200),     /* 深蓝：顶部 */
                stop: 1 rgba(10, 25, 50, 180)      /* 更深蓝：底部 */
            );
            border-radius: 12px;
        }

        QLabel {
            color: rgba(255, 255, 255, 230);
            font-size: 16px;
            font-weight: bold;
            background: transparent;   /* 关键！避免白底 */
        }
    )");

    prompt.setWindowTitle(tr("提示"));
    prompt.setFixedSize(400, 200);

    //内容布局与按钮
    QVBoxLayout* mainLayout = new QVBoxLayout(&prompt);

    QLabel* label = new QLabel(tr("请将传感器植入患者体内，点击下一步"));
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet("font-size: 16px;");
    mainLayout->addWidget(label);

    QHBoxLayout* buttonLayout = new QHBoxLayout;
    QPushButton* backButton = new QPushButton(tr("返 回"));
    QPushButton* nextButton = new QPushButton(tr("下一步"));

    // 按钮样式略...

    backButton->setFixedSize(120,45);
    backButton->setIcon(QIcon(":/image/icons8-return.png"));
    nextButton->setFixedSize(120,45);
    nextButton->setIcon(QIcon(":/image/icons8-next.png"));

    backButton->setStyleSheet(R"(
    QPushButton {
        background-color: qlineargradient(
            x1:0, y1:0, x2:0, y2:1,
            stop:0 rgba(95, 169, 246, 180),
            stop:1 rgba(49, 122, 198, 180)
        );
        border: 1px solid rgba(163, 211, 255, 0.6); /* 半透明高光边框 */
        border-radius: 6px;
        color: white;
        font-weight: bold;
        font-size: 14px;
        padding: 8px 20px;
    }

    QPushButton:pressed {
        background-color: qlineargradient(
            stop: 0 rgba(47, 106, 158, 200),
            stop: 1 rgba(31, 78, 121, 200)
        );
        padding-left: 2px;
        padding-top: 2px;
    }
    )");

    nextButton->setStyleSheet(R"(
    QPushButton {
        background-color: qlineargradient(
            stop: 0 rgba(110, 220, 145, 180),
            stop: 1 rgba(58, 170, 94, 180)
        );
        border: 1px solid rgba(168, 234, 195, 0.6);
        border-radius: 6px;
        color: white;
        font-weight: bold;
        font-size: 14px;
        padding: 8px 20px;
    }

    QPushButton:pressed {
        background-color: qlineargradient(
            stop: 0 rgba(44, 128, 73, 200),
            stop: 1 rgba(29, 102, 53, 200)
        );
        padding-left: 2px;
        padding-top: 2px;
    }
    )");

    buttonLayout->addStretch();
    buttonLayout->addWidget(backButton);
    buttonLayout->addSpacing(40);
    buttonLayout->addWidget(nextButton);
    buttonLayout->addStretch();

    mainLayout->addLayout(buttonLayout);

    QObject::connect(backButton, &QPushButton::clicked, &prompt, &QDialog::reject);
    QObject::connect(nextButton, &QPushButton::clicked, &prompt, &QDialog::accept);

    // 5. 阻塞显示
    int result = prompt.exec();

    // 6. 清除遮罩和模糊
    this->setGraphicsEffect(nullptr);
    overlay->close();
    overlay->deleteLater();

    // 7. 如果点击下一步，跳转到植入窗口
    if (result == QDialog::Accepted) {
        ImplantationSite* implantationSite = new ImplantationSite(this);
        implantationSite->setWindowFlags(Qt::Window);
        implantationSite->setFixedSize(1024, 600);
        connect(implantationSite, &ImplantationSite::returnRequested, this, [this, implantationSite]() {
            implantationSite->hide();
            this->show();
            implantationSite->deleteLater();
        });
        implantationSite->show();
        this->hide();
    }
}

void ImplantInfoWidget::closeEvent(QCloseEvent *event) {
    // 确保关闭时删除键盘
    if (currentKeyboard) {
        currentKeyboard->deleteLater();  // 删除当前键盘实例
        currentKeyboard = nullptr;  // 设置为 nullptr，防止后续使用无效指针
    }

    // 调用父类的 closeEvent 来确保默认处理
    event->accept();  // 确保继续执行关闭事件
}


void ImplantInfoWidget::showEvent(QShowEvent *event) {
    if (!eventFilterInstalled) {
        // 销毁之前的键盘实例
        if (currentKeyboard) {
            currentKeyboard->deleteLater();  // 删除键盘实例
            currentKeyboard = nullptr;
        }

        // 创建新的键盘实例
        currentKeyboard = new CustomKeyboard(this);

        // 安装事件过滤器
        this->serialInput->installEventFilter(currentKeyboard);
        this->checksumInput->installEventFilter(currentKeyboard);
        this->implantDoctorInput->installEventFilter(currentKeyboard);
        this->treatDoctorInput->installEventFilter(currentKeyboard);

        eventFilterInstalled = true;  // 设置标志为已安装
    }

    QWidget::showEvent(event);
}

