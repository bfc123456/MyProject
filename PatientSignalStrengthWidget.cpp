#include "patientsignalstrengthwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QDebug>
#include <QDialog>
#include <QAbstractItemView>
#include <QMovie>
#include "global.h"
#include "SerialStore.h"
#include <QGuiApplication>
#include <QScreen>
#include <QGraphicsBlurEffect>
#include <QSettings>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

static const char* rawOperationTips = QT_TRANSLATE_NOOP("App", R"(
<div style='font-weight: bold; font-size: 17px; margin-bottom: 10px;'>信号校准指南</div>
<div style='line-height: 1.8;'>
    <span style='color: #4CD964;'>①</span> 将传感器紧贴胸口<br>
    <span style='color: #4CD964;'>②</span> 缓慢移动寻找最佳位置<br>
    <span style='color: #4CD964;'>③</span> 信号稳定 <span style='color: #00C853; font-weight: bold;'>5秒</span> 后自动弹窗<br>
    <span style='color: #4CD964;'>④</span> 确认后开始专业测量
</div>
)");

PatientSignalStrengthWidget::PatientSignalStrengthWidget(QWidget *parent)
    : FramelessWindow(parent)
{
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int screenWidth = screenGeometry.width();
    int screenHeight = screenGeometry.height();

    float scaleX = (float)screenWidth / 1024;
    float scaleY = (float)screenHeight / 600;

    this->showFullScreen();

    QWidget *topBar = new QWidget(this);
    topBar->setStyleSheet(R"(
      QWidget {
          background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
              stop: 0 rgba(25, 50, 75, 0.9),
              stop: 1 rgba(10, 20, 30, 0.85));
          border-radius: 10px;
          border: 1px solid rgba(255, 255, 255, 0.08);
      }
      QLabel {
          color: white;
          font-weight: bold;
          font-size: 16px;
      }
    )");
    topBar->setFixedHeight(40 * scaleY);

    labelSensor= new QLabel(this);
    labelSensor->setAlignment(Qt::AlignCenter);
    labelSensor->setStyleSheet(R"(
        QLabel {
            color: white;
            font-size: 14px;
            background-color: transparent;
            border: none;
        }
    )");
    labelSensor->setText(fetchSensorIds());

    backButton = new QPushButton(this);
    connect(backButton, &QPushButton::clicked, this, &PatientSignalStrengthWidget::followReturnToLogin);
    backButton->setIcon(QIcon(":/image/icons8-return100.png"));
    backButton->setIconSize(QSize(30 * scaleX, 30 * scaleY));
    backButton->setFixedSize(65 * scaleX, 30 * scaleY);
    backButton->setFlat(true);
    backButton->setStyleSheet(R"(
        QPushButton {
            border: none;
            background-color: transparent;
            border-radius: 20px;
        }
        QPushButton:pressed {
            background-color: rgba(255, 255, 255, 0.2);
        }
    )");

    QPushButton *btnSettings = new QPushButton(this);
    btnSettings->setIcon(QIcon(":/image/icons8-shezhi.png"));
    btnSettings->setIconSize(QSize(30 * scaleX, 30 * scaleY));
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
    connect(btnSettings, &QPushButton::clicked, this, &PatientSignalStrengthWidget::openMeasureSettingsWindow);

    QHBoxLayout *topLayout = new QHBoxLayout(topBar);
    topLayout->addWidget(backButton);
    topLayout->addStretch();
    topLayout->addWidget(labelSensor);
    topLayout->addStretch();
    topLayout->addWidget(btnSettings);
    topLayout->setContentsMargins(10 * scaleX, 0, 10 * scaleX, 0);

    QWidget *middleWidget = new QWidget(this);
    middleWidget->setFixedHeight(480 * scaleY);

    QVBoxLayout *middleLayout = new QVBoxLayout(middleWidget);

    QWidget *card = new QWidget();
    card->setFixedSize(600 * scaleX, 220 * scaleY);
    card->setStyleSheet(R"(
        background-color: rgba(255, 255, 255, 0.03);
        border-radius: 20px;
        border: 1px solid rgba(255,255,255,0.1);
    )");
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect();
    shadow->setBlurRadius(30);
    shadow->setColor(QColor(0, 0, 0, 80));
    shadow->setOffset(0, 8);
    card->setGraphicsEffect(shadow);

    QLabel *animationDisplay = new QLabel();
    animationDisplay->setFixedSize(210 * scaleX, 210 * scaleY);
    animationDisplay->setAlignment(Qt::AlignCenter);

    // 加载高分辨率、医疗风格的动图
    QMovie *movie = new QMovie(":/image/animation.gif"); // 使用高质量 .gif 路径
    movie->setScaledSize(animationDisplay->size());
    movie->setSpeed(75); // 可根据实际需要调节速度（100 = 1x速率）
    animationDisplay->setMovie(movie);
    movie->start();

    // 添加卡片式背景风格
    animationDisplay->setStyleSheet(R"(
        QLabel {
            background-color: rgba(255, 255, 255, 0.04);
            border-radius: 20px;
            border: 1px solid rgba(255, 255, 255, 0.06);
            padding: 5px;
        }
    )");

    QSettings settings("MyCompany", "MyApp");
    int savedStrength = settings.value("system/signalStrength", 70).toInt();

    progress = new CircularProgressBar(this);
    progress->setThreshold(savedStrength);
    progress->setProgress(72);                     // 实时值初始
    progress->setFixedSize(210 * scaleX, 210 * scaleY);

    QHBoxLayout *cardLayout = new QHBoxLayout(card);
    cardLayout->addWidget(animationDisplay);
    cardLayout->addSpacing(20 * scaleX);
    cardLayout->addWidget(progress);

    operationTips = new QLabel(this);
    operationTips->setFixedSize(720 * scaleX, 170 * scaleY);

    operationTips->setText(
        QCoreApplication::translate("App", rawOperationTips)
    );

    operationTips->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    operationTips->setTextInteractionFlags(Qt::TextBrowserInteraction);
    operationTips->setStyleSheet(R"(
        QLabel {
            background-color: rgba(30, 41, 59, 200);
            color: #ECEFF1;
            font-size: 12px;
            border-radius: 18px;
            padding: 5px 6px;
            border: 2px solid rgba(100, 210, 150, 0.2);
            font-family: "Microsoft YaHei UI";
            line-height: 1;
        }
    )");

    QHBoxLayout *tipLayout = new QHBoxLayout();
    tipLayout->addStretch();
    tipLayout->addWidget(operationTips);
    tipLayout->addStretch();

    middleLayout->addSpacing(15 * scaleY);
    middleLayout->addWidget(card, 0, Qt::AlignCenter);
    middleLayout->addSpacing(15 * scaleY);
    middleLayout->addLayout(tipLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(80 * scaleX, 15 * scaleY, 80 * scaleX, 15 * scaleY);
    mainLayout->addWidget(topBar);
    mainLayout->addSpacing(15 * scaleY);
    mainLayout->addWidget(middleWidget);
    mainLayout->addSpacing(15 * scaleY);

    setLayout(mainLayout);

    timer = new QTimer(this);
    timer->start(100);
    connect(timer, &QTimer::timeout, this, &PatientSignalStrengthWidget::checkProgress);
    connect(this, &PatientSignalStrengthWidget::progressThresholdReached, this, &PatientSignalStrengthWidget::openMeasurementDialog);
}

PatientSignalStrengthWidget::~PatientSignalStrengthWidget() {}

void PatientSignalStrengthWidget::checkProgress() {
    if (!progress) return;

    // 从 CircularProgressBar 获取阈值
     int threshold = progress->threshold();

     // 模拟实时信号强度（目前先用常数，后续替换为真实读取值）
     int value = 72;
//    qDebug()<<"数值： "<<value;
    if (value >= threshold) {
        continuousTime++;
        if (continuousTime >= 10) {
            timer->stop(); // 停止计时器
            emit progressThresholdReached();
            qDebug()<<"信号已发出";
        }
    } else {
        // 数值回落，重置计时器
        continuousTime = 0;
    }
}

void PatientSignalStrengthWidget::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);
    if (event->type() == QEvent::LanguageChange) {
        // 系统自动发送的 LanguageChange

        operationTips->setText(
                    QCoreApplication::translate("App", rawOperationTips)
                    );
    }
    QWidget::changeEvent(event);
}

void PatientSignalStrengthWidget::openMeasurementDialog() {

    if (dlg) { dlg->close(); }           // 旧的会被 delete
    if (overlay) { overlay->deleteLater(); overlay = nullptr; }

    overlay = new QWidget(this);
    overlay->setGeometry(rect());
    overlay->setStyleSheet("background:rgba(0,0,0,100)");
    overlay->show(); overlay->raise();

    auto *blur = new QGraphicsBlurEffect;   // 不保存指针
    blur->setBlurRadius(20);
    setGraphicsEffect(blur);                // 交给 Qt 管

    dlg = new MeasurementDialog(labelSensor->text(), 80, this);
    connect(dlg,&MeasurementDialog::closePatientSignalStrengthWidget,this, &PatientSignalStrengthWidget::followReturnToLogin);
    dlg->setAttribute(Qt::WA_DeleteOnClose, true);
    dlg->setModal(true);
    dlg->adjustSize();
    dlg->move((width()-dlg->width())/2, (height()-dlg->height())/2);
    dlg->show(); dlg->raise();

    connect(dlg, &MeasurementDialog::exitOverlay, this, [this]{
        setGraphicsEffect(nullptr);         // 会自动 delete 旧 blur
        if (dlg) dlg->close();              // 触发 delete
        if (overlay) { overlay->deleteLater(); overlay=nullptr; }
    });

    // 保底：对象销毁时把指针自动清空（QPointer会自动变空，这行可选）
    connect(dlg, &QObject::destroyed, this, [this]{ dlg=nullptr; });
}

void PatientSignalStrengthWidget::openMeasureSettingsWindow() {
    if (!settingswidgetMrasure) {
        settingswidgetMrasure = new SettingsWidget(nullptr);              // 必须无父
        settingswidgetMrasure->setWindowFlags(Qt::Window);               // 设置顶层窗口

        connect(settingswidgetMrasure, &SettingsWidget::signalStrengthChanged,
                this, [this](int v) {
                    if (progress) progress->setThreshold(v);   // 立即更新显示
                    QSettings s("MyCompany","MyApp");           // 落盘
                    s.setValue("system/signalStrength", v);
                    s.sync();
                });

        // 安全关闭处理
        connect(settingswidgetMrasure, &SettingsWidget::requestDelete, this, [=]() {
            settingswidgetMrasure->deleteLater();
            settingswidgetMrasure = nullptr;
//            this->show();      //不修改 flags，只重新 show
//            this->resize(QGuiApplication::primaryScreen()->availableGeometry().size()); // 确保尺寸恢复
//            QTimer::singleShot(0, this, SLOT(update())); // 强制刷新绘制（或者用 this->update()）
        });
    }

    settingswidgetMrasure->show();
    settingswidgetMrasure->raise();
    settingswidgetMrasure->activateWindow();

//    this->hide();  // 主界面隐藏
}

QString PatientSignalStrengthWidget::fetchSensorIds() const
{
    // 取默认连接
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        qWarning() << "数据库未打开！";
        return tr("传感器ID未知");
    }

    QSqlQuery query(db);
    if (!query.exec("SELECT sensor_id FROM sensor_info")) {
        qWarning() << "查询 sensor_info 失败：" << query.lastError().text();
        return tr("传感器ID未知");
    }

    QStringList sensors;
    while (query.next()) {
        QString id = query.value(0).toString().trimmed();
        if (!id.isEmpty())
            sensors << id;
    }

    if (sensors.isEmpty())
        return tr("传感器ID查询为空");

    return sensors.join(", ");
}
