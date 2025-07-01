#include "settingswidget.h"
#include "loginwindow.h"
#include "CustomMessageBox.h"
#include "languagemanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QSizePolicy>
#include <QApplication>
#include <QDebug>
#include <QProcess>
#include <QSettings>
#include <QEvent>
#include <QImage>
#include <QPixmap>
#include <QScreen>
SettingsWidget::SettingsWidget( QWidget *parent)
    : FramelessWindow(parent)
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

    this->setObjectName("SettingsWidget");
    this->setStyleSheet(R"(
    QWidget#SettingsWidget {
        background-color: qlineargradient(
            x1: 0, y1: 1,
            x2: 1, y2: 0,
            stop: 0 rgba(6, 15, 30, 255),      /* 更暗靛蓝：左下 */
            stop: 0.5 rgba(18, 35, 65, 255),   /* 中段冷蓝 */
            stop: 1 rgba(30, 60, 100, 255)     /* 右上：深蓝灰 */
        );
    }
    )");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 顶部栏部件
    QWidget *topBar = new QWidget(this);
    topBar->setFixedHeight(50 * scaleY);
    topBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // 设置透明背景（如果你使用渐变背景）
    topBar->setStyleSheet("background-color: transparent;");

    // 系统名称 Label（左侧）
    titleLabel = new QLabel("🩺 "+ tr("医疗设备管理系统"), this);
    titleLabel->setStyleSheet("color: white; font-size: 25px; font-weight: bold;");
    titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    //设置按钮（右侧）
    QPushButton *btnclose = new QPushButton(this);
    btnclose->setIcon(QIcon(":/image/icons-close.png"));
    btnclose->setIconSize(QSize(30 * scaleX, 30 * scaleY));
    btnclose->setFlat(true);  // 去除按钮边框
    // 设置点击视觉反馈
    btnclose->setStyleSheet(R"(
        QPushButton {
            border: none;
            background-color: transparent;
            border-radius: 20px; /* 让 hover/pressed 效果是圆的 */
        }
        QPushButton:pressed {
            background-color: rgba(255, 255, 255, 0.2);
        }
    )");

    connect(btnclose, &QPushButton::clicked, this, &SettingsWidget::onBtnCloseClicked);


    //顶部栏布局
    QHBoxLayout *tittleLayout = new QHBoxLayout(topBar);
    tittleLayout->addWidget(titleLabel);
    tittleLayout->addStretch();
    tittleLayout->addWidget(btnclose);
    tittleLayout->setContentsMargins(10 * scaleX, 0, 10 * scaleX, 0);  // 左右边距

    mainLayout->addWidget(topBar);

    QWidget *topWidget = new QWidget();
    topWidget->setFixedHeight(320 * scaleY);
    topWidget->setStyleSheet(R"(
        QWidget {
            background-color: qlineargradient(
                x1: 0, y1: 0, x2: 0, y2: 1,
                stop: 0 rgba(10, 31, 56, 0.5),   /* 顶部颜色更淡 */
                stop: 1 rgba(0, 0, 0, 0.3)       /* 底部更透明 */
            );
            border-radius: 10px;
            border: 1px solid rgba(255, 255, 255, 0.06); /* 边框也变淡 */
        }
        QLabel {
            color: white;
            font-weight: bold;
            font-size: 14px;
        }
    )");

    QVBoxLayout*mainbottomLayout = new QVBoxLayout();
    mainbottomLayout->setContentsMargins(180 * scaleX, 30 * scaleY, 180 * scaleX, 30 * scaleY);
    QVBoxLayout *topLayout = new QVBoxLayout(topWidget);

    systemSettingsLabel = new QLabel(tr("系统设置"));
    systemSettingsLabel->setStyleSheet("font-family: 'Microsoft YaHei'; font-size: 20px; font-weight: bold; color: white;");
    systemSettingsLabel->setFixedHeight(40 * scaleY);
    topLayout->addWidget(systemSettingsLabel);

    // 信号强度滑块区域
    QHBoxLayout *signalLayout = new QHBoxLayout();
    signalStrengthLabel = new QLabel(tr("最小信号强度"));
    signalStrengthLabel->setFixedSize(250 * scaleX, 40 * scaleY);
    signalStrengthLabel->setStyleSheet("font-family: 'Microsoft YaHei'; font-size: 12px; font-weight: bold; color: white;");

    signalStrengthSlider = new QSlider(Qt::Horizontal);
    signalStrengthSlider->setRange(0, 100);
    signalStrengthSlider->setValue(70);
    signalStrengthSlider->setFixedWidth(160 * scaleX);

    QLabel *signalStrengthValue = new QLabel("70%");
    signalStrengthValue->setFixedSize(50 * scaleX, 40 * scaleY);
    signalStrengthValue->setAlignment(Qt::AlignCenter);
    signalStrengthValue->setStyleSheet("font-family: 'Microsoft YaHei'; font-size: 12px; font-weight: bold; color: white;");

    connect(signalStrengthSlider, &QSlider::valueChanged, [signalStrengthValue](int value) {
        signalStrengthValue->setText(QString::number(value) + "%");
    });

    modifyButton = new QPushButton(tr("修  改"));
    QPixmap pixmapModify(":/image/icons8-edit.png");

    modifyButton->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                                        stop: 0 rgba(76, 175, 80, 180),
                                        stop: 1 rgba(56, 142, 60, 180));
            border: 1px solid rgba(200, 255, 200, 0.6);  /* 光边缘 */
            border-radius: 10px;
            color: white;
            font-weight: bold;
            padding: 3px 8px;
        }

        QPushButton:pressed {
            background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                                        stop: 0 rgba(46, 125, 50, 200),
                                        stop: 1 rgba(27, 94, 32, 200));
            border: 1px solid rgba(180, 255, 180, 0.8);
        }
    )");


    QImage image3 = pixmapModify.toImage();
    image3.invertPixels();
    pixmapModify = QPixmap::fromImage(image3);
    modifyButton->setIcon(QIcon(pixmapModify));
    modifyButton->setIconSize(QSize(20 * scaleX, 20 * scaleY));
    modifyButton->setFixedSize(115 * scaleX, 40 * scaleY);

    signalLayout->addWidget(signalStrengthLabel);
    signalLayout->addWidget(signalStrengthSlider);
    signalLayout->addWidget(signalStrengthValue);
    signalLayout->addWidget(modifyButton);

    topLayout->addLayout(signalLayout);

    // 语言设置
    QHBoxLayout *languageLayout = new QHBoxLayout();
    languageLabel = new QLabel(tr("语言设置"));
    languageLabel->setStyleSheet("font-family: 'Microsoft YaHei'; font-size: 12px; font-weight: bold; color: white;");
    languageComboBox = new CustomComboBox(25 * scaleY,this);
    languageComboBox->setStyleSheet(R"(
    QComboBox {
        font-family: 'Microsoft YaHei';
        font-size: 12px;
        font-weight: bold;
        color: white;                               /* 当前显示项字体颜色 */
        background-color: rgba(30, 40, 60, 230);     /* 背景色 */
        border: 1px solid rgba(255, 255, 255, 0.2);
        border-radius: 8px;
        padding: 6px 10px;
    }

    QComboBox QAbstractItemView {
        background-color: rgba(30, 40, 60, 240);     /* 下拉菜单背景色 */
        color: white;                                /* 下拉菜单字体颜色 */
        selection-background-color: rgba(100, 160, 230, 180); /* 选中项背景色 */
        selection-color: white;                      /* 选中项字体颜色 */
        border: 1px solid rgba(255, 255, 255, 0.1);
        outline: 0;
    }
    )");

    languageComboBox->addItem(tr("中文"), "zh_CN");
    languageComboBox->addItem(tr("英语"), "en_US");
    languageComboBox->setFixedSize(150 * scaleX, 40 * scaleY);

    QString currentLang = LanguageManager::instance().currentLanguage();
    int idx = languageComboBox->findData(currentLang);
    if (idx >= 0) languageComboBox->setCurrentIndex(idx);

    connect(languageComboBox, &QComboBox::currentTextChanged, this, [=](const QString &) {
        QString selectedCode = languageComboBox->currentData().toString();
        LanguageManager::instance().switchLanguage(selectedCode);
    });


    languageLayout->addWidget(languageLabel);
    languageLayout->addWidget(languageComboBox);
    topLayout->addLayout(languageLayout);

    // 重启按钮
    QHBoxLayout *rebootLayout = new QHBoxLayout();
    rebootLabel = new QLabel(tr("系统重启"));
    rebootButton = new QPushButton(tr("重新启动"));
    QPixmap rebootIcon(":/image/icons8-reset.png");

    rebootButton->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                                        stop: 0 rgba(255, 183, 77, 180),
                                        stop: 1 rgba(255, 152, 0, 180));
            border: 1px solid rgba(255, 230, 180, 0.7);  /* 柔光边缘 */
            border-radius: 10px;
            color: white;
            font-weight: bold;
            padding: 3px 8px;
        }

        QPushButton:pressed {
            background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                                        stop: 0 rgba(255, 138, 0, 200),
                                        stop: 1 rgba(230, 81, 0, 200));
            border: 1px solid rgba(255, 220, 160, 0.8);
        }
    )");


    QImage rebootImage = rebootIcon.toImage(); rebootImage.invertPixels();
    rebootButton->setIcon(QIcon(QPixmap::fromImage(rebootImage)));
    rebootButton->setFixedSize(115 * scaleX, 40 * scaleY);
    connect(rebootButton, &QPushButton::clicked, this, &SettingsWidget::onRebootClicked);
    rebootLayout->addWidget(rebootLabel);
    rebootLayout->addWidget(rebootButton);
    topLayout->addLayout(rebootLayout);

    // 关机按钮
    QHBoxLayout *shutdownLayout = new QHBoxLayout();
    shutdownLabel = new QLabel(tr("系统关机"));
    shutdownButton = new QPushButton(tr("关  机"));
    QPixmap shutdownIcon(":/image/icons8-shutdown.png");

    shutdownButton->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                                        stop: 0 rgba(244, 67, 54, 180),
                                        stop: 1 rgba(211, 47, 47, 180));
            border: 1px solid rgba(255, 200, 200, 0.6);
            border-radius: 10px;
            color: white;
            font-weight: bold;
            padding: 3px 8px;
        }

        QPushButton:pressed {
            background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                                        stop: 0 rgba(183, 28, 28, 200),
                                        stop: 1 rgba(127, 0, 0, 200));
            border: 1px solid rgba(255, 160, 160, 0.75);
        }
    )");


    QImage shutdownImage = shutdownIcon.toImage(); shutdownImage.invertPixels();
    shutdownButton->setIcon(QIcon(QPixmap::fromImage(shutdownImage)));
    shutdownButton->setFixedSize(115 * scaleX, 40 * scaleY);
    connect(shutdownButton, &QPushButton::clicked, this, &SettingsWidget::onShutdownClicked);
    shutdownLayout->addWidget(shutdownLabel);
    shutdownLayout->addWidget(shutdownButton);
    topLayout->addLayout(shutdownLayout);

    // 添加顶部
    mainbottomLayout->addWidget(topWidget);

    // 添加底部信息
    QWidget *bottomWidget = new QWidget();

    bottomWidget->setStyleSheet(R"(
    QWidget {
        background-color: qlineargradient(
            x1: 0, y1: 0, x2: 0, y2: 1,
            stop: 0 rgba(10, 31, 56, 0.5),   /* 顶部颜色更淡 */
            stop: 1 rgba(0, 0, 0, 0.3)       /* 底部更透明 */
        );
        border-radius: 10px;
        border: 1px solid rgba(255, 255, 255, 0.06); /* 边框也变淡 */
    }
    QLabel {
        color: white;
        font-weight: bold;
        font-size: 14px;
    }
    )");

    bottomWidget->setFixedHeight(120 * scaleY);
    QVBoxLayout *bottomLayout = new QVBoxLayout(bottomWidget);

    systemInfoLabel = new QLabel(tr("系统信息"));
    systemInfoLabel->setFixedHeight(40 * scaleY);
    softwareVersionLabel1 = new QLabel(tr("   软件版本:"));
    softwareVersionLabel2 = new QLabel("V1.0.0   ");
    softwareVersionLabel2->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    QHBoxLayout *infoRow = new QHBoxLayout();
    infoRow->addWidget(softwareVersionLabel1);
    infoRow->addWidget(softwareVersionLabel2);

    bottomLayout->addWidget(systemInfoLabel);
    bottomLayout->addLayout(infoRow);
    mainbottomLayout->addWidget(bottomWidget);
    mainLayout->addLayout(mainbottomLayout);


}

SettingsWidget::~SettingsWidget() {}

void SettingsWidget::onLanguageChanged(const QString &)
{
    QString langCode = languageComboBox->currentData().toString();
    LanguageManager::instance().switchLanguage(langCode);
}

void SettingsWidget::onRebootClicked()
{
    CustomMessageBox msgBox(this, tr("确认重启"), tr("确定要重新启动应用程序吗？"),
                            {tr("是"), tr("否")}, 300);
    if (msgBox.exec() == QDialog::Accepted && msgBox.getUserResponse() == tr("是")) {
        QProcess::startDetached(QCoreApplication::applicationFilePath());
        qApp->quit();
    }
}

void SettingsWidget::onShutdownClicked()
{
    CustomMessageBox msgBox(this, tr("确认关机"), tr("确定要关闭应用程序吗？"),
                            {tr("是"), tr("否")}, 300);
    if (msgBox.exec() == QDialog::Accepted && msgBox.getUserResponse() == tr("是")) {
        qApp->quit();
    }
}

void SettingsWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        qDebug() << "SettingsWidget 收到 LanguageChange 事件";
        titleLabel->setText("🩺 "+ tr("医疗设备管理系统"));
        systemSettingsLabel->setText(tr("系统设置"));
        signalStrengthLabel->setText(tr("最小信号强度"));
        modifyButton->setText(tr("修  改"));
        languageLabel->setText(tr("语言设置"));
        rebootLabel->setText(tr("系统重启"));
        rebootButton->setText(tr("重新启动"));
        shutdownLabel->setText(tr("系统关机"));
        shutdownButton->setText(tr("关  机"));
        systemInfoLabel->setText(tr("系统信息"));
        softwareVersionLabel1->setText(tr("软件版本:"));

        languageComboBox->setItemText(0, tr("中文"));
        languageComboBox->setItemText(1, tr("英语"));
    }
    QWidget::changeEvent(event);
}

void SettingsWidget::onBtnCloseClicked(){
    this->close();
    emit requestDelete(this);  // 通知外部处理删除
}
