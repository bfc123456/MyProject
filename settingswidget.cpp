#include "SettingsWidget.h"
#include "multiuserloginwindow.h"
#include "CustomMessagebox.h"
#include "CustomCombobox.h"
#include "LanguageManager.h"
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
#include "updatemanager.h"

SettingsWidget::SettingsWidget( QWidget *parent)
    : FramelessWindow(parent)
{

    // 获取屏幕分辨率
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int screenWidth = screenGeometry.width();
    int screenHeight = screenGeometry.height();

    // 计算缩放比例
    scaleX = (float)screenWidth / 1024;
    scaleY = (float)screenHeight / 600;

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
    tittleLayout->addWidget(iconLabel);
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

    // 优先从应用属性拿（main 已经放进去），否则回退到 QSettings
    QVariant prop = qApp->property("signalStrength");
    int v = prop.isValid() ? prop.toInt()
                           : QSettings().value("system/signalStrength", 70).toInt();

    signalStrengthSlider = new QSlider(Qt::Horizontal);
    signalStrengthSlider->setRange(0, 100);
    signalStrengthSlider->setValue(v);
    signalStrengthSlider->setFixedWidth(160 * scaleX);
    signalStrengthSlider->setFixedHeight(35);

    signalStrengthSlider->setStyleSheet(R"(
        QSlider {
            background: #212121;  /* 滑动条背景色：深灰色 */
            height: 20px;  /* 增加滑动条的高度 */
            border-radius: 10px;
        }

        QSlider::handle:horizontal {
            background: #2196f3;  /* 滑块颜色：蓝色   */
            border: 2px solid #f57c00;  /* 滑块边框：浅橙色  */
            width: 50px;  /* 增加滑块宽度 */
            height: 50px;  /* 增加滑块高度 */
            border-radius: 10px;  /* 滑块圆角 */
        }

        QSlider::groove:horizontal {
            background: #2a2a2a;  /* 滑动条槽的颜色：暗灰色 */
            height: 35px;  /* 增加槽的高度 */
            border-radius: 10px;  /* 圆角 */
        }

        QSlider::add-page:horizontal {
            background: #424242;  /* 未选择区域颜色：深灰色 */
            border-radius: 10px;
        }

        QSlider::sub-page:horizontal {
            background: #64b5f6;  /* 已选择区域颜色：浅绿色 */
            border-radius: 10px;
        }
    )");

    QLabel *signalStrengthValue = new QLabel(QString::number(v) + "%");
    signalStrengthValue->setFixedSize(50 * scaleX, 40 * scaleY);
    signalStrengthValue->setAlignment(Qt::AlignCenter);
    signalStrengthValue->setStyleSheet("font-family: 'Microsoft YaHei'; font-size: 22px; font-weight: bold; color: white;");

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
            font-size: 18px;
        }

        QPushButton:pressed {
            background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                                        stop: 0 rgba(46, 125, 50, 200),
                                        stop: 1 rgba(27, 94, 32, 200));
            border: 1px solid rgba(180, 255, 180, 0.8);
        }
    )");

    // “修改”按钮：二次确认 → 写回 → 更新应用属性（全局可读）
    connect(modifyButton, &QPushButton::clicked, this, [=](){
        //从 label 里安全解析百分比
        int nv = signalStrengthValue->text().remove('%').toInt();

        // 二次确认
        CustomMessageBox ask(
            this,
            tr(" "),
            tr("将最小信号强度设置为 %1% ？\n此设置会立即生效，并在下次开机保留。").arg(nv),
            { tr("是"), tr("否") },
            300 * scaleX
        );

        if (ask.exec() != QDialog::Accepted || ask.getUserResponse() != tr("是"))
                return;

        //持久化到 QSettings（开机记住）
        QSettings s("MyCompany", "MyApp");
        s.setValue("system/signalStrength", nv);
        s.sync();

        //运行期全局可读 + 广播给其他模块
        qApp->setProperty("signalStrength", nv);
        emit signalStrengthChanged(nv);   // 其他模块 connect 这个信号即可立即生效

        //自定义“已保存”提示（也可用系统 QMessageBox::information）
        CustomMessageBox ok(
                    this,
                    tr("已保存"),
                    tr("最小信号强度已更新为 %1%。").arg(nv),
        { tr("确定") },
                    260 * scaleX
                    );
        ok.exec();

    });

    QImage image3 = pixmapModify.toImage();
    image3.invertPixels();
    pixmapModify = QPixmap::fromImage(image3);
    modifyButton->setIcon(QIcon(pixmapModify));
    modifyButton->setIconSize(QSize(15 * scaleY, 15 * scaleY));
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
        font-size: 18px;
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
            font-size: 18px;
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
            font-size: 18px;
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

    //软件更新
    QHBoxLayout *updateLayout = new QHBoxLayout();
    updateLabel = new QLabel(tr("软件更新"));
    QPushButton *updateButton = new QPushButton(tr("更  新"));
    QPixmap updateIcon(":/image/icons8-update-64.png"); // 准备一个更新图标

    updateButton->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0,y1:0,x2:0,y2:1,
                stop:0 rgba(33,150,243,180),
                stop:1 rgba(25,118,210,180));
            border: 1px solid rgba(200,220,255,0.6);
            border-radius: 10px;
            color: white;
            font-weight: bold;
            padding: 3px 8px;
            font-size: 18px;
        }
        QPushButton:pressed {
            background: qlineargradient(x1:0,y1:0,x2:0,y2:1,
                stop:0 rgba(21,101,192,200),
                stop:1 rgba(13,71,161,200));
            border: 1px solid rgba(160,190,255,0.75);
        }
    )");

    QImage updateImage = updateIcon.toImage(); updateImage.invertPixels();
    updateButton->setIcon(QIcon(QPixmap::fromImage(updateImage)));
    updateButton->setFixedSize(115 * scaleX, 40 * scaleY);

    connect(updateButton, &QPushButton::clicked, this, &SettingsWidget::onUpdateClicked);

    updateLayout->addWidget(updateLabel);
    updateLayout->addWidget(updateButton);
    topLayout->addLayout(updateLayout);


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
                            {tr("是"), tr("否")}, 300 * scaleX);
    if (msgBox.exec() == QDialog::Accepted && msgBox.getUserResponse() == tr("是")) {
        QProcess::startDetached(QCoreApplication::applicationFilePath());
        qApp->quit();
    }
}

void SettingsWidget::onShutdownClicked()
{
    CustomMessageBox msgBox(this, tr("确认关机"), tr("确定要关闭应用程序吗？"),
                            {tr("是"), tr("否")}, 300 * scaleX);
    if (msgBox.exec() == QDialog::Accepted && msgBox.getUserResponse() == tr("是")) {
        qApp->quit();
    }
}

void SettingsWidget::onUpdateClicked() {
    UpdateManager um(this);

    // 1) 查找 U 盘 UPDATE 目录
    QString usbDir;
    if (!um.hasUsbUpdate(&usbDir)) {
        CustomMessageBox(this, tr("软件更新"),
            tr("未检测到软件更新U盘，请插入后重试。"),
            { tr("确定") }, 380*scaleX).exec();
        return;
    }

    // 2) 读取 manifest
    QJsonObject manifest;
    if (!um.loadManifest(usbDir + "/manifest.json", manifest)) {
        CustomMessageBox(this, tr("软件更新"),
            tr("更新清单读取失败。"),
            { tr("确定") }, 380*scaleX).exec();
        return;
    }
    const QString version = manifest.value("version").toString();
    const QString expectSha = manifest.value("sha256").toString();

    // 3) 校验包
    if (!um.verifyPackage(usbDir + "/app_update.pkg", expectSha)) {
        CustomMessageBox(this, tr("软件更新"),
            tr("更新包校验失败。"),
            { tr("确定") }, 380*scaleX).exec();
        return;
    }

    // 4) 用户确认
    if (CustomMessageBox(this, tr("软件更新"),
        tr("检测到新版本 %1，是否继续更新？").arg(version),
        { tr("取消"), tr("继续") }, 420*scaleX).exec() != 1) {
        return;
    }

    // 5) 复制到本地 staging
    QString staging;
    if (!um.stageUpdate(usbDir, staging)) {
        CustomMessageBox(this, tr("软件更新"),
            tr("复制更新包失败。"),
            { tr("确定") }, 380*scaleX).exec();
        return;
    }

    // 6) 重启应用以应用更新（pending 标记）
    if (CustomMessageBox(this, tr("软件更新"),
        tr("将重启应用以应用更新，是否现在重启？"),
        { tr("取消"), tr("继续") }, 420*scaleX).exec() != 1) {
        return;
    }
    um.markPendingAndRestart(staging);
}

void SettingsWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        qDebug() << "SettingsWidget 收到 LanguageChange 事件";
        titleLabel->setText(tr("医疗设备管理系统"));
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
