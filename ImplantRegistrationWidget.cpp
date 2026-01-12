#include "implantregistrationwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QDebug>
#include <QGraphicsBlurEffect>
#include <QEvent>
#include <QMessageBox>
#include "custommessagebox.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include "closeonlywindow.h"
#include <QGuiApplication>
#include <QScreen>
#include "medicallogger.h"


ImplantRegistrationWidget::ImplantRegistrationWidget(QWidget *parent)
    : FramelessWindow(parent)
{
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int screenWidth = screenGeometry.width();
    int screenHeight = screenGeometry.height();

    // 计算缩放比例
    scaleX = (float)screenWidth / 1024;
    scaleY = (float)screenHeight / 600;

    // 设置窗口初始大小
    this->resize(1024 * scaleX, 600 * scaleY);  // 设置为基于目标分辨率的大小
    this->setObjectName("ImplantRegistrationWidget");
    this->setStyleSheet(R"(
    QWidget#ImplantRegistrationWidget {
        background-color: qlineargradient(
            x1: 0, y1: 1,
            x2: 1, y2: 0,
            stop: 0 rgba(6, 15, 30, 255),      /* 更暗靛蓝：左下 */
            stop: 0.5 rgba(18, 35, 65, 255),   /* 中段冷蓝 */
            stop: 1 rgba(30, 60, 100, 255)     /* 右上：深蓝灰 */
        );
    }

    QLabel {
        color: white;
        font-weight: bold;
        font-size: 16px;
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

    topBar->setFixedHeight(50*scaleY);


    titleLabel = new QLabel(tr("新植入物"), this);
    titleLabel->setFixedSize(160*scaleX, 35*scaleY);
    titleLabel->setAutoFillBackground(false);                   // 不自动填充背景
    titleLabel->setAttribute(Qt::WA_TranslucentBackground);     // 启用透明背景
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: white;");
    titleLabel->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);

    QPushButton *btnSettings = new QPushButton(this);
    connect(btnSettings, &QPushButton::clicked, this, &ImplantRegistrationWidget::openSettingsWindow);  // 连接点击事件到槽函数
    btnSettings->setIcon(QIcon(":/image/icons8-shezhi.png"));
    btnSettings->setIconSize(QSize(24*scaleX, 24*scaleY));
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
    topLayout->setContentsMargins(10*scaleX, 0*scaleY, 10*scaleX, 0*scaleY);

    // 表单部分
    serialLabel = new QLabel(tr("传感器序列号"));
    serialLabel->setFixedSize(180*scaleX, 40*scaleY);
    serialLabel->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    serialInput = new QLineEdit();
    serialInput->setFixedSize(400*scaleX, 50*scaleY);
    serialInput->setPlaceholderText(tr("请输入传感器序列号"));

    checksumLabel = new QLabel(tr("校准码"));
    checksumLabel->setFixedSize(180*scaleX, 40*scaleY);
    checksumLabel->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    checksumInput = new QLineEdit();
    checksumInput->setFixedSize(400*scaleX, 50*scaleY);
    checksumInput->setPlaceholderText(tr("请输入校准码"));

    implantDoctorLabel = new QLabel(tr("植入医生(选填)"));
    implantDoctorLabel->setFixedSize(180*scaleX,40*scaleY);
    implantDoctorLabel->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    implantDoctorInput = new QLineEdit();
    implantDoctorInput->setFixedSize(400*scaleX, 50*scaleY);
    implantDoctorInput->setPlaceholderText(tr("请输入植入医生姓名(选填)"));

    treatDoctorLabel = new QLabel(tr("治疗医生(选填)"));
    treatDoctorLabel->setFixedSize(180*scaleX, 40*scaleY);
    treatDoctorLabel->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);

    treatDoctorInput = new QLineEdit();
    treatDoctorInput->setFixedSize(400*scaleX, 50*scaleY);
    treatDoctorInput->setPlaceholderText(tr("请输入治疗医生姓名(选填)"));

    dateLabel = new QLabel(tr("植入日期"));
    dateLabel->setFixedSize(180*scaleX, 40*scaleY);
    dateLabel->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);

    implantDateInput = new TouchDateEdit();
    implantDateInput->setDate(QDate(2025, 1, 1));
    implantDateInput->setFixedSize(400*scaleX, 50*scaleY);

    // 应用触控样式
    implantDateInput->applyTouchStyle(scaleX, scaleY,
                                      380,
                                      300,
                                      15,
                                      14 );

//    currentKeyboard = CustomKeyboard::instance(this);

//    // 给每个 QLineEdit 注册一次偏移
//    currentKeyboard->registerEdit(serialInput);
//    currentKeyboard->registerEdit(checksumInput);
//    currentKeyboard->registerEdit(implantDoctorInput);
//    currentKeyboard->registerEdit(treatDoctorInput);

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
    formWidget->setFixedHeight(350*scaleY);
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
    connect(backButton, &QPushButton::clicked, this,&ImplantRegistrationWidget::implantReturnLogin);

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
    continueButton->setIcon(QIcon(":/image/icons8-next.png"));
    connect(continueButton, &QPushButton::clicked, this, [this]() {

            bool ok = insertNewSensor();  // 返回 true／false   记得改回来
            if (!ok) {
               showImplantationSiteWidget(m_serial);
               MedicalLogger::instance()->writeLog(
                   "ImplantationSiteWidget",
                   MedicalLogger::LOG_INFO,
                   "Entering ImplantationSiteWidget interface",
                   " ",   // 目前没有登录时用占位符
                   "UI"                 // 这里是 UI 相关操作
               );
            } else {
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

                //创建信息对话框
                CustomMessageBox dlg(this,tr("错误"),tr("上传失败，请再次检查输入"), { tr("确定") },350 *scaleX);
                int result = dlg.exec();
                // 判断用户的选择并记录日志
                if (result == QDialog::Accepted) {
                    // 用户点击了“确定”
                    MedicalLogger::instance()->writeLog(
                        "Upload",
                        MedicalLogger::LOG_INFO,
                        "User clicked 'Confirm' after upload failure",
                        " ",  // 暂时占位，后续替换为当前用户
                        "UI"
                    );
                } else {
                    // 用户点击了“取消”或者关闭了对话框
                    MedicalLogger::instance()->writeLog(
                        "Upload",
                        MedicalLogger::LOG_INFO,
                        "User dismissed the error message without confirming",
                        " ",  // 暂时占位，后续替换为当前用户
                        "UI"
                    );
                }

                //清空输入内容
                serialInput->clear();
                checksumInput->clear();
                implantDoctorInput->clear();
                treatDoctorInput->clear();
                implantDateInput->clear();

                // 清除遮罩和模糊
                this->setGraphicsEffect(nullptr);
                overlay->close();
                overlay->deleteLater();
            }
    });

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

    backButton->setFixedSize(120*scaleX, 40*scaleY);
    continueButton->setFixedSize(120*scaleX, 40*scaleY);

    QWidget *buttonWidget = new QWidget();
    buttonWidget->setFixedHeight(70*scaleY);
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

    QHBoxLayout *buttonLayout = new QHBoxLayout(buttonWidget);
    buttonLayout->setContentsMargins(20*scaleX, 10*scaleY, 20*scaleX, 10*scaleY);
    buttonLayout->addWidget(backButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(continueButton);

    buttonLayout->setAlignment(Qt::AlignCenter);

    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(80*scaleX, 15*scaleY, 80*scaleX, 15*scaleY);
    mainLayout->addWidget(topBar);
    mainLayout->addSpacing(15*scaleY);
    mainLayout->addWidget(formWidget);
    mainLayout->addSpacing(30*scaleY);
    mainLayout->addWidget(buttonWidget);
    mainLayout->addSpacing(15*scaleY);
    setLayout(mainLayout);
}

ImplantRegistrationWidget::~ImplantRegistrationWidget() {

}

void ImplantRegistrationWidget::showImplantationSiteWidget(const QString &serial)
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

    CustomMessageBox dlg(
        this,
        tr("提示"),
        tr("请将传感器植入患者体内，点击下一步"),
        { tr("返 回"), tr("下一步") },
        350*scaleX   // 宽度
    );

    // 5. 阻塞显示
    dlg.exec();
    QString clickedBtn = dlg.getUserResponse();

    // 6. 清除遮罩和模糊
    this->setGraphicsEffect(nullptr);
    overlay->close();
    overlay->deleteLater();

    // 7. 如果点击下一步，跳转到植入窗口
    if (clickedBtn == tr("下一步")) {
        ImplantationSite* implantationSite = new ImplantationSite(this,serial);
//        qDebug() << "ImplantationSite constructed.";
        implantationSite->setWindowFlags(Qt::Window);
        implantationSite->setFixedSize(1024*scaleX, 600*scaleY);
        connect(implantationSite, &ImplantationSite::returnRequested, this, [this, implantationSite]() {
            implantationSite->hide();
            this->show();
            MedicalLogger::instance()->writeLog(
                "implantationSite",
                MedicalLogger::LOG_INFO,
                "Returning to ImplantRegistrationWidget interface",
                " ",   // 目前没有登录时用占位符
                        "UI"                 // 这里是 UI 相关操作
                        );
            implantationSite->deleteLater();
            //            qDebug() << "ImplantationSite deleteLater triggered.";
        });
        implantationSite->show();
        MedicalLogger::instance()->writeLog(
                    "implantationSite",
                    MedicalLogger::LOG_AUDIT,
                    "ImplantationSite window opened (user accepted)",
                    "UnknownOperator",
                    "UI"
                    );
        QTimer::singleShot(200, this, [this]() {
            this->hide();            // 隐藏当前窗口
        });
    }else if (clickedBtn == tr("返 回")){
        emit implantReturnLogin();

        // 日志：用户取消植入操作
        MedicalLogger::instance()->writeLog(
                    "implantationSite",
                    MedicalLogger::LOG_AUDIT,
                    "ImplantationSite canceled by user",
                    " ",
                    "UI"
                    );
    }
}

bool ImplantRegistrationWidget::insertNewSensor()
{
    //先做合法性检查：序列号 + 校准码 必须在对照表里存在
    m_serial  = serialInput->text().trimmed();
    QString calib  = checksumInput  ->text().trimmed();
    QSqlQuery checkQ;
    checkQ.prepare(R"(
        SELECT COUNT(*)
          FROM sensor_serial_number_check
         WHERE sensor_id=:id AND calibration_code=:code
    )");
    checkQ.bindValue(":id",   m_serial);
    checkQ.bindValue(":code", calib);

    //执行语句失败或未检查到内容皆表示失败
    if (!checkQ.exec() || !checkQ.next()) {
        QString err = "校验表查询失败: " + checkQ.lastError().text();
        MedicalLogger::instance()->writeLog(
            "Database",
            MedicalLogger::LOG_ERROR,
            err,
            " ",
            m_serial
        );
        return false;
    }
    if (checkQ.value(0).toInt() == 0) {
        QString err = "序列号/校准码在对照表中不存在，禁止注册！";
        MedicalLogger::instance()->writeLog(
            "Database",
            MedicalLogger::LOG_WARN,
            err,
            " ",
            m_serial
        );
        return false;
    }

    //再检查该传感器是不是已经在主表里注册过了
    QSqlQuery existQ;
    existQ.prepare("SELECT COUNT(*) FROM sensor_info WHERE sensor_id=:id");
    existQ.bindValue(":id", m_serial);
    if (!existQ.exec() || !existQ.next()) {
        QString err = "主表查询失败: " + existQ.lastError().text();
        MedicalLogger::instance()->writeLog(
            "Database",
            MedicalLogger::LOG_ERROR,
            err,
            " ",
            m_serial
        );
        return false;
    }
    if (existQ.value(0).toInt() > 0) {
        QString err = "序列号已注册过，禁止重复添加！";
        MedicalLogger::instance()->writeLog(
            "Database",
            MedicalLogger::LOG_WARN,
            err,
            " ",
            m_serial
        );
        return false;
    }

    //真正做 INSERT
    QSqlQuery insertQ;
    insertQ.prepare(R"(
        INSERT INTO sensor_info (
            sensor_id,
            calibration_code,
            plant_doctor,
            treatment_doctor,
            planting_date,
            location
        ) VALUES (
            :id,
            :code,
            :pdoc,
            :tdoc,
            :date,
            :loc
        )
    )");

    // 读取选填并决定是否写 NULL
    const QString pdoc = implantDoctorInput->text().trimmed();
    const QString tdoc = treatDoctorInput->text().trimmed();
    const QString date = implantDateInput->text().trimmed();

    insertQ.bindValue(":id",    m_serial);
    insertQ.bindValue(":code",  calib);
    insertQ.bindValue(":pdoc",  pdoc.isEmpty()?QVariant():QVariant(pdoc));
    insertQ.bindValue(":tdoc",  tdoc.isEmpty()?QVariant():QVariant(tdoc));
    // SQLite 中可以用 ISO 格式字符串存日期
    insertQ.bindValue(":date",  date);
    insertQ.bindValue(":loc",  QVariant());

    if (!insertQ.exec()) {
        QString errMsg = insertQ.lastError().text();
        MedicalLogger::instance()->writeLog(
            "Database",
            MedicalLogger::LOG_ERROR,                          // 数据库错误
            QString("Failed to insert new sensor: %1").arg(errMsg),
            " ",                                 // 当前操作员（现用 UnknownOperator）
            "DB"
        );
        return false;
    }

    MedicalLogger::instance()->writeLog(
        "Database",
        MedicalLogger::LOG_AUDIT,                              // 审计日志，关键数据操作
        QString("New sensor inserted successfully: serial=%1").arg(m_serial),
        " ",
        "DB"
    );
    return true;
}

void ImplantRegistrationWidget::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);
    if (event->type() == QEvent::LanguageChange) {
        // 系统自动发送的 LanguageChange
        titleLabel->setText(tr("新植入物"));
        serialLabel->setText(tr("传感器序列号"));
        serialInput->setPlaceholderText(tr("请输入传感器序列号"));
        checksumLabel->setText(tr("校准码"));
        checksumInput->setPlaceholderText(tr("请输入校准码"));
        implantDoctorLabel->setText(tr("植入医生"));
        implantDoctorInput->setPlaceholderText(tr("请输入植入医生姓名"));
        treatDoctorLabel->setText(tr("治疗医生"));
        treatDoctorInput->setPlaceholderText(tr("请输入治疗医生姓名"));
        dateLabel->setText(tr("植入日期"));
        backButton->setText(tr("返回"));
        continueButton->setText(tr("继续"));
    }
}
