#include "followupform.h"
#include "implantmonitor.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QDebug>
#include <QDialog>
#include <QAbstractItemView>
#include "global.h"

FollowUpForm::FollowUpForm(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle(tr("回访管理界面"));
    setFixedSize(1024, 600);

    // 顶部栏
    QWidget *topBar = new QWidget(this);
    topBar->setStyleSheet(R"(
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
          font-size: 16px;
      }
    )");

    topBar->setFixedHeight(50);

    QLabel *titleLabel = new QLabel(tr("回访"), this);
    titleLabel->setFixedSize(160, 35);
    titleLabel->setAutoFillBackground(false);                   // 不自动填充背景
    titleLabel->setAttribute(Qt::WA_TranslucentBackground);     // 启用透明背景
    titleLabel->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);

    QPushButton *btnSettings = new QPushButton(this);
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
    connect(btnSettings, &QPushButton::clicked, this, &FollowUpForm::openSettingsWindow);  // 连接点击事件到槽函数
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

    QLabel *locationLabel = new QLabel(tr("植入位置"));
    locationLabel->setFixedSize(160, 40);
    locationLabel->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    //  创建 QComboBox 对象
    selectcomboBox = new QComboBox(this);
    selectcomboBox->setFixedSize(400, 50);
    // 添加下拉选项“左”与“右”
    selectcomboBox->addItem(tr("左"));
    selectcomboBox->addItem(tr("右"));

    selectcomboBox->setMinimumWidth(150);

    //设置默认选中项
    selectcomboBox->setCurrentIndex(0);

    // 监听用户选择的变化，连接信号槽
    connect(selectcomboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [=](int index){
        // 根据 index 实现相应的业务逻辑
        qDebug() << "当前选择的选项索引为:" << index;
    });

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
    formLayout->addWidget(locationLabel, 4, 1);
    formLayout->addWidget(selectcomboBox, 5, 1);

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
    QLineEdit, QDateEdit,QComboBox {
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
    connect(backButton, &QPushButton::clicked, this, &FollowUpForm::followReturnToLogin);
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
    buttonLayout->setContentsMargins(20, 0, 20, 0);
    buttonLayout->addWidget(backButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(continueButton);

    QWidget *buttonWidget = new QWidget();
    buttonWidget->setFixedHeight(70);

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
    // 连接按钮
    connect(backButton, &QPushButton::clicked, this, [this](){
        this->close();                      // 关闭当前表单（FollowUpForm）
        if (globalLoginWindowPointer) {
            globalLoginWindowPointer->show();         // 再次显示主界面
        }
    });
    connect(continueButton, &QPushButton::clicked, this, &FollowUpForm::showImplantionSite);

    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(80, 15, 80, 15);
    mainLayout->addWidget(topBar);
    mainLayout->addWidget(formWidget);
    mainLayout->addSpacing(30);
    mainLayout->addWidget(buttonWidget);
    mainLayout->addSpacing(45);
    setLayout(mainLayout);
}

FollowUpForm::~FollowUpForm() {}

void FollowUpForm::showImplantionSite(){
    ImplantationSite* implantationsite = new ImplantationSite(this);
    implantationsite->setWindowFlags(Qt::Window);
    implantationsite->setFixedSize(1024, 600);
    implantationsite->show();
    this->hide();
}

void FollowUpForm::closeEvent(QCloseEvent *event) {
    // 确保关闭时删除键盘
    if (currentKeyboard) {
        currentKeyboard->deleteLater();  // 删除当前键盘实例
        currentKeyboard = nullptr;  // 设置为 nullptr，防止后续使用无效指针
    }

    // 调用父类的 closeEvent 来确保默认处理
    event->accept();  // 确保继续执行关闭事件
}


void FollowUpForm::showEvent(QShowEvent *event) {
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

//void FollowUpForm::validateForm() {
//    // 获取所有输入框的内容
//    QString serial = serialInput->text().trimmed();      // 传感器序列号
//    QString calib = checksumInput->text().trimmed();        // 校准码
//    QString doctor = implantDoctorInput->text().trimmed();      // 植入医生
//    QString treatdoctor = treatDoctorInput->text().trimmed();   //治疗医生
//    QString date = implantDateInput->text().trimmed();  // 植入日期
//    QString location = selectcomboBox->currentText().trimmed();     // 植入位置（假设这是一个 QComboBox）

//    // 检查是否所有字段都有值
//    bool isValid = !serial.isEmpty() && !calib.isEmpty() && !doctor.isEmpty() &&
//                   !date.isEmpty() && !location.isEmpty();

//    // 如果所有输入框有效，启用继续按钮，否则禁用继续按钮
//    continueButton->setEnabled(isValid);
//}

