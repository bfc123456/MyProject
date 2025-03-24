#include "settingswidget.h"
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QComboBox>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QSizePolicy>
#include <QApplication>
#include <QDebug>
#include <QProcess>
#include <QSettings>
#include "CustomMessageBox.h"


SettingsWidget::SettingsWidget(MainWindow *parentWindow, QWidget *parent)
    : QWidget(parent), mainWindow(parentWindow)
{
    // ✅ 读取存储的语言
    QSettings settings("MyCompany", "MyApp");
    QString savedLanguage = settings.value("language", "zh_CN").toString(); // 默认中文

    setWindowTitle(tr("设置界面"));
    this->setFixedSize(1024, 600);  // 固定窗口大小
    this->setStyleSheet(R"(
        /* 设置整个 QWidget 的背景颜色 */
        QWidget {
            background-color: #181818;  /* 深色背景 */
            color: white;
        }

    )");



    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout();
    setLayout(mainLayout);

    // 设置主布局的左上右下页边距（这里可以根据需求修改数值）
    mainLayout->setContentsMargins(230, 50, 230, 50);

    // 上部分布局（系统设置）
    QWidget *topWidget = new QWidget();
    topWidget->setFixedHeight(320); // 设置固定高度为200
    topWidget->setStyleSheet(R"(
        QWidget {
            background-color: #2E2E2E;
            border-radius: 12px;
            border: 2px solid #444444;
        }
    )");
    QVBoxLayout *topLayout = new QVBoxLayout(topWidget);

    // 添加标题“系统设置”
    QHBoxLayout *systemSettingsLayout = new QHBoxLayout();
    systemSettingsLabel = new QLabel(tr("系统设置"));
    systemSettingsLabel->setStyleSheet("font-family: 'Microsoft YaHei'; font-size: 20px; font-weight: bold; color: white;");
    systemSettingsLabel->setFixedHeight(40); // 设置label的固定高度为40
    systemSettingsLayout->addWidget(systemSettingsLabel);
    topLayout->addLayout(systemSettingsLayout);

    // 创建一个水平布局用于最小信号强度、滑动条、修改按钮
    QHBoxLayout *signalLayout = new QHBoxLayout();
    signalStrengthLabel = new QLabel(tr("最小信号强度"));
    signalStrengthLabel->setFixedHeight(40);
    signalStrengthLabel->setStyleSheet("font-family: 'Microsoft YaHei'; font-size: 12px; font-weight: bold; color: #B0B0B0;");
    signalStrengthSlider = new QSlider(Qt::Horizontal);
    signalStrengthSlider->setRange(0, 100);     //默认为0-100
    signalStrengthSlider->setValue(70);         // 默认值设为 70%
    signalStrengthSlider->setStyleSheet(
        "QSlider::groove:horizontal {"
        "    border: 2px solid #333;"  /* 滑轨边框 */
        "    height: 6px;"             /* 轨道厚度 */
        "    background: #444;"        /* 轨道颜色 */
        "    border-radius: 3px;"      /* 圆角 */
        "}"

        "QSlider::sub-page:horizontal {"
        "    background: qlineargradient(x1: 0, x2: 1, stop: 0 #3E8EFF, stop: 1 #00D8FF);"
        "    border-radius: 3px;"
        "}"

        "QSlider::handle:horizontal {"
        "    background: #007BFF;"     /* 滑块默认颜色 */
        "    border: 2px solid #0056b3;"
        "    width: 14px;"             /* 滑块宽度 */
        "    height: 14px;"            /* 滑块高度 */
        "    margin: -4px 0;"
        "    border-radius: 7px;"      /* 滑块圆角 */
        "}"

        "QSlider::handle:horizontal:hover {"
        "    background: #00D8FF;"     /* 悬浮时颜色 */
        "    border: 2px solid #007BFF;"
        "}"

        "QSlider::handle:horizontal:pressed {"
        "    background: #0056b3;"     /* 按下时颜色 */
        "    width: 16px;"             /* 按下时放大 */
        "    height: 16px;"
        "}"
    );

    signalStrengthSlider->setFixedWidth(160);
    QLabel *signalStrengthValue = new QLabel("70%");  // 显示当前值
    signalStrengthValue->setFixedWidth(50);
    signalStrengthValue->setFixedHeight(40);
    signalStrengthValue->setAlignment(Qt::AlignCenter);
    signalStrengthValue->setStyleSheet("font-family: 'Microsoft YaHei'; font-size: 12px; font-weight: bold; color: #B0B0B0;");
    connect(signalStrengthSlider, &QSlider::valueChanged, [signalStrengthValue](int value) {
        signalStrengthValue->setText(QString::number(value) + "%");
    });
    modifyButton = new QPushButton(tr("修  改"));
    QPixmap pixmapModify(":image/icons8-edit.png");
    QImage image3 = pixmapModify.toImage();
    image3.invertPixels(); // 反转颜色
    pixmapModify = QPixmap::fromImage(image3);
    modifyButton->setIcon(QIcon(pixmapModify));
    modifyButton->setIconSize(QSize(20, 20));
    modifyButton->setFixedWidth(115);
    modifyButton->setFixedHeight(40);
    modifyButton->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #0073E6, stop: 1 #005BB5);
            color: white;
            border: 2px solid #004C99;
            border-radius: 8px;
            padding: 6px 16px;
            font-family: 'Microsoft YaHei';
            font-weight: bold;
            font-size: 14px;
            text-align: center;
        }

        QPushButton:pressed {
            background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #005BB5, stop: 1 #004C99);
            border-style: inset;
        }

        QPushButton:disabled {
            background: #444;
            color: #aaa;
            border: 2px solid #666;
        }
    )");


    signalLayout->addWidget(signalStrengthLabel);

    // 添加间隔：在控件之间插入间隔
    QSpacerItem *spacer1 = new QSpacerItem(10, 50, QSizePolicy::Fixed, QSizePolicy::Minimum);  // 设置间隔大小
    signalLayout->addItem(spacer1);  // 添加间隔到布局中

    signalLayout->addWidget(signalStrengthSlider);
    signalLayout->addWidget(signalStrengthValue);

    // 添加间隔：在控件之间插入间隔
    QSpacerItem *spacer2 = new QSpacerItem(10, 50, QSizePolicy::Fixed, QSizePolicy::Minimum);  // 设置间隔大小
    signalLayout->addItem(spacer2);  // 添加间隔到布局中

    signalLayout->addWidget(modifyButton);

    // 创建一个水平布局用于语言设置和下拉选择框
    QHBoxLayout *languageLayout = new QHBoxLayout();
    languageLabel = new QLabel(tr("语言设置"));
    languageLabel->setStyleSheet("font-family: 'Microsoft YaHei'; font-size: 12px; font-weight: bold; color: #B0B0B0;");
    languageComboBox = new QComboBox(this);
    languageComboBox->addItem("简体中文", "zh_CN");
    languageComboBox->setStyleSheet("font-family: 'Microsoft YaHei'; font-size: 12px; font-weight: bold; color: #B0B0B0;");
    languageComboBox->addItem("English","en_US");
    languageComboBox->setStyleSheet("font-family: 'Microsoft YaHei'; font-size: 12px; font-weight: bold; color: #B0B0B0;");

    // ✅ 让 `QComboBox` 选中正确的语言
    int index = languageComboBox->findData(savedLanguage);
    if (index != -1) {
        languageComboBox->setCurrentIndex(index);
    }


    languageComboBox->setFixedWidth(150);
    languageComboBox->setFixedHeight(40);

    languageComboBox->setStyleSheet(
        "QComboBox {"
        "    background-color: #2E2E2E;"  // **统一背景颜色**
        "    color: gray;"  // **字体颜色设为灰色**
        "    font-family: SimHei;"  // **设置字体为黑体**
        "    border: 2px solid #444444;"  // **统一边框颜色**
        "    border-radius: 8px;"  // **增加圆角**
        "    padding: 5px 10px;"
        "    font-size: 14px;"
        "    qproperty-iconSize: 20px 20px;"  // **确保下拉箭头大小**
        "}"
        "QComboBox::drop-down {"
        "    background-color: #444444;"  // **下拉按钮区域颜色**
        "    border-left: 1px solid #555;"
        "    width: 30px;"  // **下拉箭头区域加宽**
        "    subcontrol-origin: padding;"
        "    subcontrol-position: right center;"  // **让箭头在右侧垂直居中**
        "}"
        "QComboBox::down-arrow {"
        "    image: url(:/image/icons-pulldown.png);"  // **使用图片作为箭头**
        "    width: 16px;"  // **控制箭头大小**
        "    height: 16px;"
        "    margin-right: 8px;"  // **让箭头与右侧保持适当距离**
        "}"
        "QComboBox QAbstractItemView {"
        "    background-color: #222222;"  // **下拉选项背景**
        "    selection-background-color: #007BFF;"  // **选中颜色**
        "    color: gray;"  // **下拉列表字体颜色设为灰色**
        "    font-family: SimHei;"  // **下拉列表字体也使用黑体**
        "    border: 1px solid #444444;"
        "    border-radius: 6px;"
        "}"
    );










    connect(languageComboBox, &QComboBox::currentTextChanged, this, &SettingsWidget::onLanguageChanged);


    languageLayout->addWidget(languageLabel);
    languageLayout->addWidget(languageComboBox);

    // 创建一个水平布局用于系统重启按钮
    QHBoxLayout *rebootLayout = new QHBoxLayout();
    rebootLabel = new QLabel(tr("系统重启"));
    rebootLabel->setStyleSheet("font-family: 'Microsoft YaHei'; font-size: 12px; font-weight: bold; color: #B0B0B0;");
    rebootButton = new QPushButton(tr("重新启动"));
    QPixmap pixmapReboot(":image/icons8-reset.png");
    QImage image = pixmapReboot.toImage();
    image.invertPixels(); // 反转颜色
    pixmapReboot = QPixmap::fromImage(image);
    rebootButton->setIcon(QIcon(pixmapReboot));
    rebootButton->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #FFA726, stop: 1 #FF8C00);
            color: white;
            border: 2px solid #D2691E;
            border-radius: 8px;
            padding: 6px 16px;
            font-family: 'Microsoft YaHei';
            font-weight: bold;
            font-size: 14px;
            text-align: center;
        }

        QPushButton:pressed {
            background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #E65100, stop: 1 #D2691E);
            border-style: inset;
        }

        QPushButton:disabled {
            background: #555;
            color: #bbb;
            border: 2px solid #666;
        }
    )");

    rebootButton->setIconSize(QSize(15, 15));
    rebootButton->setFixedWidth(115);
    rebootButton->setFixedHeight(40);
    rebootLayout->addWidget(rebootLabel);
    rebootLayout->addWidget(rebootButton);

    // 将按钮点击信号与槽连接
      connect(rebootButton, &QPushButton::clicked, this, &SettingsWidget::onRebootClicked);

    // 创建一个水平布局用于关机按钮
    QHBoxLayout *shutdownLayout = new QHBoxLayout();
    shutdownLabel = new QLabel(tr("系统关机"));
    shutdownLabel->setStyleSheet("font-family: 'Microsoft YaHei'; font-size: 12px; font-weight: bold; color: #B0B0B0;");
    shutdownButton = new QPushButton(tr("关  机"));
    QPixmap pixmapShutdown(":image/icons8-shutdown.png");
    QImage image2 = pixmapShutdown.toImage();
    image2.invertPixels(); // 反转颜色
    pixmapShutdown = QPixmap::fromImage(image2);
    shutdownButton->setIcon(QIcon(pixmapShutdown));
    shutdownButton->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #FF4C4C, stop: 1 #D32F2F);
            color: white;
            border: 2px solid #B71C1C;
            border-radius: 8px;
            padding: 6px 16px;
            font-family: 'Microsoft YaHei';
            font-weight: bold;
            font-size: 14px;
            text-align: center;
        }

        QPushButton:pressed {
            background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #D32F2F, stop: 1 #B71C1C);
            border-style: inset;
        }

        QPushButton:disabled {
            background: #555;
            color: #bbb;
            border: 2px solid #666;
        }
    )");


    shutdownButton->setIconSize(QSize(20, 20));
    shutdownButton->setFixedWidth(115);
    shutdownButton->setFixedHeight(40);
    shutdownLayout->addWidget(shutdownLabel);
    shutdownLayout->addWidget(shutdownButton);
    // 连接关机按钮的点击信号到退出槽
    connect(shutdownButton, &QPushButton::clicked, this, &SettingsWidget::onShutdownClicked);


    // 将这些布局添加到topLayout
    topLayout->addLayout(signalLayout);
    topLayout->addLayout(languageLayout);
    topLayout->addLayout(rebootLayout);
    topLayout->addLayout(shutdownLayout);

    // 创建一个QWidget作为底部布局容器
    QWidget *bottomWidget = new QWidget();
    bottomWidget->setFixedHeight(120);
    QVBoxLayout *bottomLayout = new QVBoxLayout(bottomWidget);  // 用于底部部分的布局
    bottomWidget->setLayout(bottomLayout);
    bottomWidget->setStyleSheet(R"(
        QWidget {
            background-color: #2E2E2E;
            border-radius: 12px;
            border: 2px solid #444444;
            padding: 0px;
        }
    )");

    // 系统信息
    QHBoxLayout *systemInformationLayout = new QHBoxLayout();
    systemInfoLabel = new QLabel(tr("系统信息"));
    systemInfoLabel->setStyleSheet("font-family: 'Microsoft YaHei'; font-size: 20px; font-weight: bold; color: white;");
    systemInfoLabel->setFixedHeight(40); // 设置label的固定高度为40
    systemInformationLayout->addWidget(systemInfoLabel);
    bottomLayout->addLayout(systemInformationLayout);

    QHBoxLayout *systemInfoLayout = new QHBoxLayout();
    softwareVersionLabel1 = new QLabel(tr("   软件版本:"));
    softwareVersionLabel1->setStyleSheet("font-family: 'Microsoft YaHei'; font-size: 12px; font-weight: bold; color: #B0B0B0;");
    softwareVersionLabel2 = new QLabel("V1.0.0   ");
    softwareVersionLabel2->setAlignment(Qt::AlignRight| Qt::AlignVCenter);
    softwareVersionLabel2->setStyleSheet("font-family: 'Microsoft YaHei'; font-size: 12px; font-weight: bold; color: #B0B0B0;");

    systemInfoLayout->addWidget(softwareVersionLabel1);
    systemInfoLayout->addWidget(softwareVersionLabel2);

    bottomLayout->addLayout(systemInfoLayout);

    // 将控件添加到主布局中
    mainLayout->addWidget(topWidget);
    // 添加间隔：使用 QSpacerItem 创建固定的垂直间隔
    QSpacerItem *spacer = new QSpacerItem(20, 15, QSizePolicy::Minimum, QSizePolicy::Fixed);  // 设置间隔大小
    mainLayout->addItem(spacer);
    mainLayout->addWidget(bottomWidget);
}

SettingsWidget::~SettingsWidget() {}

// 重新启动按钮的槽函数
void SettingsWidget::onRebootClicked() {
    // ✅ 创建自定义消息框
    CustomMessageBox msgBox(this, tr("确认重启"), tr("确定要重新启动应用程序吗？"),
                            {tr("是"), tr("否")}, 300);

    // ✅ 显示对话框，并等待用户响应
    if (msgBox.exec() == QDialog::Accepted && msgBox.getUserResponse() == tr("是")) {
        qDebug() << "Rebooting application...";

        // 获取当前程序的路径
        QString program = QCoreApplication::applicationFilePath();

        // 启动一个新的进程，重新运行当前程序
        QProcess::startDetached(program);

        // 退出当前程序
        qApp->quit();
    } else {
        qDebug() << "用户取消了重启操作";
    }
}

void SettingsWidget::onShutdownClicked() {
    // ✅ 创建自定义消息框
    CustomMessageBox msgBox(this, tr("确认关机"), tr("确定要关闭应用程序吗？"),
                            {tr("是"), tr("否")}, 300);

    // ✅ 显示对话框，并等待用户响应
    if (msgBox.exec() == QDialog::Accepted && msgBox.getUserResponse() == tr("是")) {
        qDebug() << "Shutting down application...";
        qApp->quit();  // **退出应用**
    } else {
        qDebug() << "用户取消了关机操作";
    }
}


void SettingsWidget::onLanguageChanged(const QString &) {
    QString languageCode = languageComboBox->currentData().toString();  // 获取 "zh_CN" 或 "en_US"
    qDebug() << "User selected LanguageCode:" << languageCode;

    //存储到 QSettings，下次启动自动加载
    QSettings settings("MyCompany", "MyApp");
    settings.setValue("language", languageCode);

    //先从 Qt 中移除旧翻译器
    static QTranslator translator;
    qApp->removeTranslator(&translator);

    //加载新的翻译文件
    if (translator.load(":/translations/app_" + languageCode + ".qm")) {
        qApp->installTranslator(&translator);
    }

    //通知 MainWindow 语言已更改
    if (mainWindow) {
        mainWindow->changeLanguage(languageCode);
    }

    //触发 Qt 的语言切换事件，更新 UI
    QEvent event(QEvent::LanguageChange);
    QApplication::sendEvent(this, &event);
}


void SettingsWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        qDebug() << "SettingsWidget语言切换事件触发！";

        // **更新翻译**
        setWindowTitle(tr("设置界面"));
        systemSettingsLabel->setText(tr("系统设置"));
        signalStrengthLabel->setText(tr("最小信号强度"));
        modifyButton->setText(tr("修  改"));
        languageLabel->setText(tr("语言设置"));
        rebootLabel->setText(tr("系统重启"));
        rebootButton->setText(tr("重新启动"));
        shutdownLabel->setText(tr("系统关机"));
        shutdownButton->setText(tr("关  机"));
        systemInfoLabel->setText(tr("系统信息"));
        softwareVersionLabel1 ->setText(tr("软件版本:"));

        qDebug() << "SettingsWidgetUI 语言切换完成！";
    }
    QWidget::changeEvent(event);
}


