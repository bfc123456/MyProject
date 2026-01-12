
/********************************************************************************/
/* 文件名    : SettingsWidget.cpp                                                 */
/* 功能      : 系统设置界面（包括信号强度、语言切换、系统操作等）               */
/* 版本      : 1.0.0                                                              */
/* 作者      :                                                         */
/* 日期      : 2025-12-26                                                       */
/* 说明      : 该文件实现了系统设置界面，提供信号强度调整、语言切换及其他系统操作功能   */
/********************************************************************************/

//1) Project Headers
#include "settingswidget.h"
#include "multiuserloginwindow.h"
#include "custommessagebox.h"
#include "customcombobox.h"
#include "languagemanager.h"
#include "bluroverlayguard.h"
#include "updatemanager.h"

//2) Qt Headers
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

//构造函数
SettingsWidget::SettingsWidget( QWidget *parent)
    : FramelessWindow(parent)
{

    // 获取屏幕分辨率
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int iScreenWidth = screenGeometry.width();
    int iScreenHeight = screenGeometry.height();

    // 计算缩放比例
    m_fScaleX = (float)iScreenWidth / 1024;
    m_fScaleY = (float)iScreenHeight / 600;

    // 设置窗口初始大小
    this->resize(1024 * m_fScaleX, 600 * m_fScaleY);  // 设置为基于目标分辨率的大小

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

    QVBoxLayout *pMainLayout = new QVBoxLayout(this);

    // 顶部栏部件
    QWidget *pTopBarWidget = new QWidget(this);
    pTopBarWidget->setFixedHeight(50 * m_fScaleY);
    pTopBarWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // 设置透明背景（如果你使用渐变背景）
    pTopBarWidget->setStyleSheet("background-color: transparent;");

    // 系统名称 Label（左侧）
    QLabel *pConLabel = new QLabel(this);
    QPixmap pix(":/image/icons8-tingzhen.png");
    // 缩放到合适大小，比如 24×24
    pix = pix.scaled(24 * m_fScaleX, 24 * m_fScaleY, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    pConLabel->setPixmap(pix);
    pConLabel->setFixedSize(pix.size());

    m_pTitleLabel = new QLabel(tr("医疗设备管理系统"), this);
    m_pTitleLabel->setStyleSheet("color: white; font-size: 25px; font-weight: bold;");
    m_pTitleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    //设置按钮（右侧）
    QPushButton *pCloseBtn = new QPushButton(this);
    pCloseBtn->setIcon(QIcon(":/image/icons-close.png"));
    pCloseBtn->setIconSize(QSize(30 * m_fScaleX, 30 * m_fScaleY));
    pCloseBtn->setFlat(true);  // 去除按钮边框
    // 设置点击视觉反馈
    pCloseBtn->setStyleSheet(R"(
        QPushButton {
            border: none;
            background-color: transparent;
            border-radius: 20px; /* 让 hover/pressed 效果是圆的 */
        }
        QPushButton:pressed {
            background-color: rgba(255, 255, 255, 0.2);
        }
    )");

    connect(pCloseBtn, &QPushButton::clicked, this, &SettingsWidget::SlotOnBtnCloseClicked);


    //顶部栏布局
    QHBoxLayout *pTittleLayout = new QHBoxLayout(pTopBarWidget);
    pTittleLayout->addWidget(pConLabel);
    pTittleLayout->addWidget(m_pTitleLabel);
    pTittleLayout->addStretch();
    pTittleLayout->addWidget(pCloseBtn);
    pTittleLayout->setContentsMargins(10 * m_fScaleX, 0, 10 * m_fScaleX, 0);  // 左右边距

    pMainLayout->addWidget(pTopBarWidget);

    QWidget *pTopWidget = new QWidget();
    pTopWidget->setFixedHeight(320 * m_fScaleY);
    pTopWidget->setStyleSheet(R"(
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

    QVBoxLayout*pMainpBottomLayout = new QVBoxLayout();
    pMainpBottomLayout->setContentsMargins(180 * m_fScaleX, 30 * m_fScaleY, 180 * m_fScaleX, 30 * m_fScaleY);
    QVBoxLayout *pTopLayout = new QVBoxLayout(pTopWidget);

    m_pSystemSettingsLabel = new QLabel(tr("系统设置"));
    m_pSystemSettingsLabel->setStyleSheet("font-family: 'Microsoft YaHei'; font-size: 20px; font-weight: bold; color: white;");
    m_pSystemSettingsLabel->setFixedHeight(40 * m_fScaleY);
    pTopLayout->addWidget(m_pSystemSettingsLabel);

    // 信号强度滑块区域
    QHBoxLayout *pSignalLayout = new QHBoxLayout();
    m_pSignalStrengthLabel = new QLabel(tr("最小信号强度"));
    m_pSignalStrengthLabel->setFixedSize(250 * m_fScaleX, 40 * m_fScaleY);
    m_pSignalStrengthLabel->setStyleSheet("font-family: 'Microsoft YaHei'; font-size: 12px; font-weight: bold; color: white;");

    // 优先从应用属性拿（main 已经放进去），否则回退到 QSettings
    QVariant prop = qApp->property("signalStrength");
    int v = prop.isValid() ? prop.toInt()
                           : QSettings().value("system/signalStrength", 70).toInt();

    m_pSignalStrengthSlider = new QSlider(Qt::Horizontal);
    m_pSignalStrengthSlider->setRange(0, 100);
    m_pSignalStrengthSlider->setValue(v);
    m_pSignalStrengthSlider->setFixedWidth(160 * m_fScaleX);
    m_pSignalStrengthSlider->setFixedHeight(35);

    m_pSignalStrengthSlider->setStyleSheet(R"(
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

    QLabel *pSigStrengthValueLabel = new QLabel(QString::number(v) + "%");
    pSigStrengthValueLabel->setFixedSize(50 * m_fScaleX, 40 * m_fScaleY);
    pSigStrengthValueLabel->setAlignment(Qt::AlignCenter);
    pSigStrengthValueLabel->setStyleSheet("font-family: 'Microsoft YaHei'; font-size: 22px; font-weight: bold; color: white;");

    connect(m_pSignalStrengthSlider, &QSlider::valueChanged, [pSigStrengthValueLabel](int value) {
        pSigStrengthValueLabel->setText(QString::number(value) + "%");
    });

    m_pModifyButton = new QPushButton(tr("修  改"));
    QPixmap pixmapModify(":/image/icons8-edit.png");
    m_pModifyButton->setStyleSheet(R"(
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
    connect(m_pModifyButton, &QPushButton::clicked, this, [=](){
        //从 label 里安全解析百分比
        int nv = pSigStrengthValueLabel->text().remove('%').toInt();

        // 二次确认
        CustomMessageBox ask(
            this,
            tr(" "),
            tr("将最小信号强度设置为 %1% ？\n此设置会立即生效，并在下次开机保留。").arg(nv),
            { tr("是"), tr("否") },
            300 * m_fScaleX
        );

        if (ask.exec() != QDialog::Accepted || ask.GetUserResponse() != tr("是"))
                return;

        //持久化到 QSettings（开机记住）
        QSettings s("MyCompany", "MyApp");
        s.setValue("system/signalStrength", nv);
        s.sync();

        //运行期全局可读 + 广播给其他模块
        qApp->setProperty("signalStrength", nv);
        emit SigStrengthChanged(nv);   // 其他模块 connect 这个信号即可立即生效

        //自定义“已保存”提示（也可用系统 QMessageBox::information）
        CustomMessageBox ok(
                    this,
                    tr("已保存"),
                    tr("最小信号强度已更新为 %1%。").arg(nv),
        { tr("确定") },
                    260 * m_fScaleX
                    );
        ok.exec();

    });

    QImage image3 = pixmapModify.toImage();
    image3.invertPixels();
    pixmapModify = QPixmap::fromImage(image3);
    m_pModifyButton->setIcon(QIcon(pixmapModify));
    m_pModifyButton->setIconSize(QSize(15 * m_fScaleY, 15 * m_fScaleY));
    m_pModifyButton->setFixedSize(115 * m_fScaleX, 40 * m_fScaleY);

    pSignalLayout->addWidget(m_pSignalStrengthLabel);
    pSignalLayout->addWidget(m_pSignalStrengthSlider);
    pSignalLayout->addWidget(pSigStrengthValueLabel);
    pSignalLayout->addWidget(m_pModifyButton);

    pTopLayout->addLayout(pSignalLayout);

    // 语言设置
    QHBoxLayout *pLanguageLayout = new QHBoxLayout();
    m_pLanguageLabel = new QLabel(tr("语言设置"));
    m_pLanguageLabel->setStyleSheet("font-family: 'Microsoft YaHei'; font-size: 12px; font-weight: bold; color: white;");
    m_pLanguageComboBox = new CustomComboBox(25 * m_fScaleY,this);
    m_pLanguageComboBox->setStyleSheet(R"(
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

    m_pLanguageComboBox->addItem(tr("中文"), "zh_CN");
    m_pLanguageComboBox->addItem(tr("英语"), "en_US");
    m_pLanguageComboBox->setFixedSize(150 * m_fScaleX, 40 * m_fScaleY);


    QString strCurrentLang = LanguageManager::instance().currentLanguage();
    int idx = m_pLanguageComboBox->findData(strCurrentLang);
    if (idx >= 0) m_pLanguageComboBox->setCurrentIndex(idx);

    // 3. 连接选择变化信号
    // 设置界面中语言选择框的连接代码（添加详细日志）
    connect(m_pLanguageComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [=](int index) {
        Q_UNUSED(index);
        // 获取选中项的语言代码和显示文本
        QString strSelectedCode = m_pLanguageComboBox->currentData().toString();
        QString strSelectedText = m_pLanguageComboBox->currentText(); // 显示给用户的文本（如"中文"、"English"）

        // 关键日志：打印用户选择的详细信息
        qDebug() << "[SettingsWidget] 语言选择框索引变化，开始处理...";
        qDebug() << "[SettingsWidget]   选中的显示文本：" << strSelectedText;
        qDebug() << "[SettingsWidget]   对应的语言代码：" << strSelectedCode;

        if (strSelectedCode.isEmpty()) {
            qDebug() << "[SettingsWidget]   ❌ 选中项语言代码为空，不执行切换";
            return;
        }

        // 检查是否与当前语言一致（提前规避无效切换）
        QString strCurrentLang = LanguageManager::instance().currentLanguage();
        if (strSelectedCode == strCurrentLang) {
            qDebug() << "[SettingsWidget]   提示：选中语言与当前语言一致（" << strCurrentLang << "），无需切换";
            return;
        }

        // 调用管理器切换语言，打印调用前日志
        qDebug() << "[SettingsWidget]   准备调用 LanguageManager::switchLanguage(" << strSelectedCode << ")";
        LanguageManager::instance().switchLanguage(strSelectedCode);
        qDebug() << "[SettingsWidget]   调用切换函数完成，等待管理器处理...";
    });

    pLanguageLayout->addWidget(m_pLanguageLabel);
    pLanguageLayout->addWidget(m_pLanguageComboBox);
    pTopLayout->addLayout(pLanguageLayout);

    // 重启按钮
    QHBoxLayout *pRebootLayout = new QHBoxLayout();
    m_pRebootLabel = new QLabel(tr("系统重启"));
    m_pRebootButton = new QPushButton(tr("重新启动"));
    QPixmap rebootIcon(":/image/icons8-reset.png");

    m_pRebootButton->setStyleSheet(R"(
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
    m_pRebootButton->setIcon(QIcon(QPixmap::fromImage(rebootImage)));
    m_pRebootButton->setFixedSize(115 * m_fScaleX, 40 * m_fScaleY);
    connect(m_pRebootButton, &QPushButton::clicked, this, &SettingsWidget::SlotOnRebootClicked);
    pRebootLayout->addWidget(m_pRebootLabel);
    pRebootLayout->addWidget(m_pRebootButton);
    pTopLayout->addLayout(pRebootLayout);

    // 关机按钮
    QHBoxLayout *pShutdownLayout = new QHBoxLayout();
    m_pShutdownLabel = new QLabel(tr("系统关机"));
    m_pShutdownButton = new QPushButton(tr("关  机"));
    QPixmap shutdownIcon(":/image/icons8-shutdown.png");

    m_pShutdownButton->setStyleSheet(R"(
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
    m_pShutdownButton->setIcon(QIcon(QPixmap::fromImage(shutdownImage)));
    m_pShutdownButton->setFixedSize(115 * m_fScaleX, 40 * m_fScaleY);
    connect(m_pShutdownButton, &QPushButton::clicked, this, &SettingsWidget::SlotOnShutdownClicked);
    pShutdownLayout->addWidget(m_pShutdownLabel);
    pShutdownLayout->addWidget(m_pShutdownButton);

    //软件更新
    QHBoxLayout *pUpdateLayout = new QHBoxLayout();
    m_pUpdateLabel = new QLabel(tr("软件更新"));
    m_pUpdateButton = new QPushButton(tr("更  新"));
    QPixmap updateIcon(":/image/icons8-update-64.png"); // 准备一个更新图标

    m_pUpdateButton->setStyleSheet(R"(
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
    m_pUpdateButton->setIcon(QIcon(QPixmap::fromImage(updateImage)));
    m_pUpdateButton->setFixedSize(115 * m_fScaleX, 40 * m_fScaleY);

    connect(m_pUpdateButton, &QPushButton::clicked, this, &SettingsWidget::SlotOnUpdateClicked);

    pUpdateLayout->addWidget(m_pUpdateLabel);
    pUpdateLayout->addWidget(m_pUpdateButton);
    pTopLayout->addLayout(pUpdateLayout);


    pTopLayout->addLayout(pShutdownLayout);

    // 添加顶部
    pMainpBottomLayout->addWidget(pTopWidget);

    // 添加底部信息
    QWidget *pBottomWidget = new QWidget();

    pBottomWidget->setStyleSheet(R"(
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

    pBottomWidget->setFixedHeight(120 * m_fScaleY);
    QVBoxLayout *pBottomLayout = new QVBoxLayout(pBottomWidget);

    m_pSystemInfoLabel = new QLabel(tr("系统信息"));
    m_pSystemInfoLabel->setFixedHeight(40 * m_fScaleY);
    m_pSoftwareVersionLabel1 = new QLabel(tr("   软件版本:"));
    m_pSoftwareVersionLabel2 = new QLabel("V1.0.0   ");
    m_pSoftwareVersionLabel2->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    QHBoxLayout *pInfoRowLayout = new QHBoxLayout();
    pInfoRowLayout->addWidget(m_pSoftwareVersionLabel1);
    pInfoRowLayout->addWidget(m_pSoftwareVersionLabel2);

    pBottomLayout->addWidget(m_pSystemInfoLabel);
    pBottomLayout->addLayout(pInfoRowLayout);
    pMainpBottomLayout->addWidget(pBottomWidget);
    pMainLayout->addLayout(pMainpBottomLayout);
    Retranslate(); // 首次设置文案
}

//析构函数
SettingsWidget::~SettingsWidget() {}

//void SettingsWidget::SlotOInLanguageChanged(const QString &)
//{
//    QString langCode = m_pLanguageComboBox->currentData().toString();
//    LanguageManager::instance().switchLanguage(langCode);
//}

/***********************************************************************************************
 * FUNC    : SlotOInLanguageChanged
 * IN      : langCode (QString) - 当前选择的语言代码
 * OUT     : None
 * RETURN  : void
 * AUTHOR  : 2025-12-26 Create by lxh for SettingsWidget class
 * NOTE    : 触发语言切换事件，更新语言设置
 ************************************************************************************************/
void SettingsWidget::SlotOInLanguageChanged(const QString &)
{
    QString langCode = m_pLanguageComboBox->currentData().toString();
    qDebug() << "[SlotOInLanguageChanged槽] 触发语言切换，代码：" << langCode;
    LanguageManager::instance().switchLanguage(langCode);
}

/***********************************************************************************************
 * FUNC    : SlotOnRebootClicked
 * IN      : None
 * OUT     : None
 * RETURN  : void
 * AUTHOR  : 2025-12-26 Create by lxh for SettingsWidget class
 * NOTE    : 触发系统重启操作，弹出确认对话框，用户确认后重启应用程序
 ************************************************************************************************/
void SettingsWidget::SlotOnRebootClicked()
{
    // 1. 进入函数自动加背景模糊 + 遮罩
    BlurOverlayGuard blurGuard(this, /*blurRadius=*/20, /*alpha=*/100);

    CustomMessageBox msgBox(this, tr("确认重启"), tr("确定要重新启动应用程序吗？"),
                            {tr("是"), tr("否")}, 300 * m_fScaleX);
    msgBox.exec();
    // 3. 根据用户选择执行关机
    if (msgBox.GetUserResponse() == tr("是")) {
        QProcess::startDetached(QCoreApplication::applicationFilePath());
        qApp->quit();
    }
}

/***********************************************************************************************
 * FUNC    : SlotOnShutdownClicked
 * IN      : None
 * OUT     : None
 * RETURN  : void
 * AUTHOR  : 2025-12-26 Create by lxh for SettingsWidget class
 * NOTE    : 触发系统关机操作，弹出确认对话框，用户确认后关闭应用程序
 ************************************************************************************************/

void SettingsWidget::SlotOnShutdownClicked()
{
    // 1. 进入函数自动加背景模糊 + 遮罩
    BlurOverlayGuard blurGuard(this, /*blurRadius=*/20, /*alpha=*/100);

    // 2. 弹出确认对话框
    CustomMessageBox msgBox(
        this,
        tr("确认关机"),
        tr("确定要关闭应用程序吗？"),
        { tr("是"), tr("否") },
        300 * m_fScaleX
    );

    msgBox.exec();  // 阻塞等待用户点击

    // 3. 根据用户选择执行关机
    if (msgBox.GetUserResponse() == tr("是")) {
        qApp->quit();
    }
    // 函数结束时 blurGuard 析构 -> 自动清除遮罩和模糊
}

/***********************************************************************************************
 * FUNC    : SlotOnUpdateClicked
 * IN      : None
 * OUT     : None
 * RETURN  : void
 * AUTHOR  : 2025-12-26 Create by lxh for SettingsWidget class
 * NOTE    : 触发软件更新操作，检查U盘更新包并执行更新
 ************************************************************************************************/

void SettingsWidget::SlotOnUpdateClicked()
{
    // 使用之前封装好的类
    BlurOverlayGuard blurGuard(this, /*blurRadius=*/20, /*alpha=*/120);

    UpdateManager um(this);

    // 1) 查找 U 盘 UPDATE 目录
    QString usbDir;
    if (!um.hasUsbUpdate(&usbDir)) {
        CustomMessageBox(this, tr("软件更新"),
            tr("未检测到软件更新U盘，请插入后重试。"),
            { tr("确定") }, 380*m_fScaleX).exec();
        return;
    }

    // 2) 读取 manifest
    QJsonObject manifest;
    if (!um.loadManifest(usbDir + "/manifest.json", manifest)) {
        CustomMessageBox(this, tr("软件更新"),
            tr("更新清单读取失败。"),
            { tr("确定") }, 380*m_fScaleX).exec();
        return;
    }

    const QString version   = manifest.value("version").toString();
    const QString expectSha = manifest.value("sha256").toString();

    // 3) 校验包
    if (!um.verifyPackage(usbDir + "/app_update.pkg", expectSha)) {
        CustomMessageBox(this, tr("软件更新"),
            tr("更新包校验失败。"),
            { tr("确定") }, 380*m_fScaleX).exec();
        return;
    }

    // 4) 用户确认
    if (CustomMessageBox(this, tr("软件更新"),
        tr("检测到新版本 %1，是否继续更新？").arg(version),
        { tr("取消"), tr("继续") }, 420*m_fScaleX).exec() != 1) {
        return;
    }

    // 5) 复制到本地 staging
    QString staging;
    if (!um.stageUpdate(usbDir, staging)) {
        CustomMessageBox(this, tr("软件更新"),
            tr("复制更新包失败。"),
            { tr("确定") }, 380*m_fScaleX).exec();
        return;
    }

    // 6) 重启确认
    if (CustomMessageBox(this, tr("软件更新"),
        tr("将重启应用以应用更新，是否现在重启？"),
        { tr("取消"), tr("继续") }, 420*m_fScaleX).exec() != 1) {
        return;
    }

    // 应用即将退出，不需要担心 blurGuard 析构
    um.markPendingAndRestart(staging);
}

/***********************************************************************************************
 * FUNC    : ChangeEvent
 * IN      : event (QEvent*) - 事件对象
 * OUT     : None
 * RETURN  : void
 * AUTHOR  : 2025-12-26 Create by lxh for SettingsWidget class
 * NOTE    : 语言变化时调用，更新界面控件文本
 ************************************************************************************************/

void SettingsWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        Retranslate();
    }
    QWidget::changeEvent(event);
}

/***********************************************************************************************
 * FUNC    : SlotOnBtnCloseClicked
 * IN      : None
 * OUT     : None
 * RETURN  : void
 * AUTHOR  : 2025-12-26 Create by lxh for SettingsWidget class
 * NOTE    : 关闭设置界面并发送销毁请求信号
 ************************************************************************************************/

void SettingsWidget::SlotOnBtnCloseClicked(){
    this->close();
    emit SigRequestDelete(this);  // 通知外部处理删除
}

/***********************************************************************************************
 * FUNC    : Retranslate
 * IN      : None
 * OUT     : None
 * RETURN  : void
 * AUTHOR  : 2025-12-26 Create by lxh for SettingsWidget class
 * NOTE    : 刷新界面控件的文本（根据当前语言）
 ************************************************************************************************/

void SettingsWidget::Retranslate()
{
    qDebug() << "SettingsWidget 收到 LanguageChange 事件";

    if (m_pTitleLabel)            m_pTitleLabel->setText(tr("医疗设备管理系统"));
    if (m_pSystemSettingsLabel)   m_pSystemSettingsLabel->setText(tr("系统设置"));
    if (m_pSignalStrengthLabel)   m_pSignalStrengthLabel->setText(tr("最小信号强度"));
    if (m_pModifyButton)          m_pModifyButton->setText(tr("修  改"));
    if (m_pLanguageLabel)         m_pLanguageLabel->setText(tr("语言设置"));
    if (m_pUpdateLabel)           m_pUpdateLabel->setText(tr("软件更新"));
    if (m_pUpdateButton)          m_pUpdateButton->setText(tr("更新"));
    if (m_pRebootLabel)           m_pRebootLabel->setText(tr("系统重启"));
    if (m_pRebootButton)          m_pRebootButton->setText(tr("重新启动"));
    if (m_pShutdownLabel)         m_pShutdownLabel->setText(tr("系统关机"));
    if (m_pShutdownButton)        m_pShutdownButton->setText(tr("关  机"));
    if (m_pSystemInfoLabel)       m_pSystemInfoLabel->setText(tr("系统信息"));
    if (m_pSoftwareVersionLabel1) m_pSoftwareVersionLabel1->setText(tr("软件版本:"));

    if (m_pLanguageComboBox) {
        int keep = m_pLanguageComboBox->currentIndex();  // 保留选择
        if (m_pLanguageComboBox->count() > 0) m_pLanguageComboBox->setItemText(0, tr("中文"));
        if (m_pLanguageComboBox->count() > 1) m_pLanguageComboBox->setItemText(1, tr("英语"));
        if (keep >= 0 && keep < m_pLanguageComboBox->count())
            m_pLanguageComboBox->setCurrentIndex(keep);
    }

}
