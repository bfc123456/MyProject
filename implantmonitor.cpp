#include "ImplantMonitor.h"
#include "DeviceAcquisitionWorker.h"
#include "MeasurementDataProcessor.h"
#include "CardiacoutputDialog.h"
#include "SettingsWidget.h"
#include <QScreen>
#include <QDateTime>
#include <QSettings>
#include <QMetaObject>
#include <QGuiApplication>
#include <QGraphicsBlurEffect>
#include <qwt_scale_widget.h>
#include <QFileDialog>
#include <QMessageBox>
//测试

// 提取重复样式表，统一维护
const QString DARK_CARD_STYLE = R"(
QWidget {
    background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 rgba(25, 50, 75, 0.9), stop:1 rgba(10, 20, 30, 0.85));
    border-radius: 10px;
    border: 1px solid rgba(255, 255, 255, 0.08);
}
)";

ImplantMonitor::ImplantMonitor(QWidget *parent, const QString &sensorId)
    : FramelessWindow(parent), m_serial(sensorId)
{
    // 1. 屏幕缩放计算
    QScreen *screen = QGuiApplication::primaryScreen();
    int screenWidth = screen->geometry().width();
    int screenHeight = screen->geometry().height();
    scaleX = (float)screenWidth / 1024;
    scaleY = (float)screenHeight / 600;
    setFixedSize(1024*scaleX, 600*scaleY);
    setObjectName("Implantonitor");
    setStyleSheet(R"(
    QWidget#Implantonitor {
        background-color: qlineargradient(x1:0, y1:1, x2:1, y2:0,
            stop:0 rgba(6, 15, 30, 255), stop:0.5 rgba(18, 35, 65, 255), stop:1 rgba(30, 60, 100, 255));
    }
    )");

    // 2. 主布局初始化
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(30*scaleX, 15*scaleY, 30*scaleX, 15*scaleY);

    // 3. 顶部栏（标题 + 设置按钮）
    QWidget *topWidget = new QWidget(this);
    topWidget->setObjectName("TopBar");
    topWidget->setStyleSheet(R"(
        QWidget#TopBar {
            background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 rgba(25, 50, 75, 0.9), stop:1 rgba(10, 20, 30, 0.85));
            border-radius: 10px;
            border: 1px solid rgba(255, 255, 255, 0.08);
        }
        QLabel { color: white; font-weight: bold; font-size: 16px; }
    )");
    topWidget->setFixedHeight(50*scaleY);

    titleLabel = new QLabel(tr("新植入物"));
    idLabel = new QLabel(sensorId);
    titleLabel->setStyleSheet("background: transparent;");
    idLabel->setStyleSheet("background: transparent;");
    titleLabel->setFixedWidth(120*scaleX);
    idLabel->setFixedWidth(120*scaleX);
    titleLabel->setAlignment(Qt::AlignCenter);
    idLabel->setAlignment(Qt::AlignCenter);

    QPushButton *settingsBtn = new QPushButton(this);
    settingsBtn->setIcon(QIcon(":/image/icons8-shezhi.png"));
    settingsBtn->setIconSize(QSize(24*scaleX, 24*scaleY));
    settingsBtn->setStyleSheet("border: none; background: transparent; border-radius: 20px;");
    connect(settingsBtn, &QPushButton::clicked, this, &ImplantMonitor::OpenSettingsRequested);

    QHBoxLayout *topLayout = new QHBoxLayout(topWidget);
    topLayout->addWidget(titleLabel, 0, Qt::AlignLeft);
    topLayout->addWidget(idLabel, 1, Qt::AlignCenter);
    topLayout->addSpacing(96*scaleX);
    topLayout->addWidget(settingsBtn, 0, Qt::AlignRight);

    // 4. 中间波形区
    QWidget *middleWidget = new QWidget(this);
    middleWidget->setFixedHeight(350*scaleY);
    middleWidget->setStyleSheet(DARK_CARD_STYLE);
    plot = new ModernWavePlot(this);
    plot->setStyleSheet("background: transparent; border: none; color:white;");
    plot->setLineColor(QColor(100, 180, 255));
    plot->setFillColor(QColor(40, 120, 200, 30), -1);
    plot->setAxisScale(QwtPlot::yLeft, 0, 500);
    plot->setAxisScale(QwtPlot::xBottom, 0, 30);
    plot->setMinimumHeight(200*scaleY);

    QwtText xTitle("Time (s)"), yTitle("Pressure (mmHg)");
    plot->axisWidget(QwtPlot::xBottom)->setTitle(xTitle);
    plot->axisWidget(QwtPlot::yLeft)->setTitle(yTitle);

    QVBoxLayout *middleLayout = new QVBoxLayout(middleWidget);
    middleLayout->addWidget(plot);
    middleLayout->setContentsMargins(50*scaleX, 30*scaleY, 50*scaleX, 30*scaleY);

    // 5. 底部控件区（信号、位置、数据、按钮）
    QHBoxLayout *bottomLayout = new QHBoxLayout();

    // 左一：信号进度
    QWidget *signalWidget = new QWidget;
    signalWidget->setFixedHeight(120*scaleY);
    signalWidget->setStyleSheet(DARK_CARD_STYLE);
    QVBoxLayout *signalLayout = new QVBoxLayout(signalWidget);

    QSettings settings("MyCompany", "MyApp");
    int signalThresh = settings.value("system/signalStrength", 70).toInt();
    progressBar = new CircularProgressBar(this);
    progressBar->setFixedSize(65*scaleX, 65*scaleY);
    progressBar->setThreshold(signalThresh);
    progressBar->setProgress(90);
    signalLayout->addWidget(progressBar, 0, Qt::AlignCenter);


    //左二：植入位置显示
    QWidget *posWidget = new QWidget(parent);
    posWidget->setObjectName("posWidget");
    posWidget->setFixedSize(180*scaleX, 120*scaleY);
    posWidget->setStyleSheet(R"(
                                    QWidget#posWidget {
                                    border: none;
                                    /* 用 border-image 拉伸背景 */
                                    border-image: url(:/image/newbody.png);
                                    }
                                    )");
    //在它上面放一个 QLabel 来显示“L”或“R”
    QLabel *sideLabel = new QLabel(posWidget);
    sideLabel->setFixedSize(40*scaleX, 40*scaleY);
    sideLabel->setAlignment(Qt::AlignCenter);
    sideLabel->setStyleSheet(R"(
                             QLabel {
                             background-color: rgba(33, 150, 243, 0.85);  /* #2196F3 + 85% 不透明度 */
                             color: white;
                             font-size: 18px;
                             border-radius: 6px;
                             }
                             )");

    QString loc = DatabaseManager::instance().getLocationBySensorId(m_serial);

    m_isLeft = (loc == "left");
    //    qDebug()<<" "<<m_isLeft;

    //根据左右来移动到正确位置，比如右侧偏上
    if (m_isLeft) {
        sideLabel->setText(tr("左"));
        // 左侧居中偏左
        sideLabel->move( 40, 2*(posWidget->height() - sideLabel->height())/3);
    } else {
        sideLabel->setText(tr("右"));
        // 右侧居中偏右
        sideLabel->move(
                    (posWidget->width() - sideLabel->width() - 40),
                    2*(posWidget->height() - sideLabel->height())/3
                    );
    }
    sideLabel->show();

    // 右二：数据显示
    QWidget *dataWidget = new QWidget;
    dataWidget->setFixedSize(260*scaleX, 120*scaleY);
    dataWidget->setStyleSheet(DARK_CARD_STYLE);
    QHBoxLayout *dataLayout = new QHBoxLayout(dataWidget);

    bpVal = new QLabel(tr("血压\n0.00/0.00"), this);
    bpVal->setFixedSize(110*scaleX,45*scaleY);
    avgVal = new QLabel(tr("平均\n0.00"), this);
    avgVal->setFixedSize(110*scaleX,45*scaleY);
    hrVal = new QLabel(tr("心率\n0.00"), this);
    hrVal->setFixedSize(110*scaleX,45*scaleY);
    statisticsbtn = new QPushButton(tr("读数记录"));

    QString cardLabelStyle = R"(
        QLabel {
            background-color: qlineargradient(
                x1: 0, y1: 0, x2: 0, y2: 1,
                stop: 0 rgba(255, 255, 255, 20),
                stop: 1 rgba(255, 255, 255, 10)
            );
            color: white;
            font-size: 13px;
            font-weight: bold;
            border: 1px solid rgba(255, 255, 255, 50);
            border-radius: 10px;
            padding: 2px;
        }
    )";
    bpVal->setStyleSheet(cardLabelStyle);
    avgVal->setStyleSheet(cardLabelStyle);
    hrVal->setStyleSheet(cardLabelStyle);

    statisticsbtn->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 rgba(180,180,255,40), stop:1 rgba(120,130,200,30));
            color: white; font-size: 14px; border: 1px solid rgba(255,255,255,40);
            border-radius: 8px; padding: 2px 5px;
        }
        QPushButton:pressed { background: rgba(100,120,200,50); }
    )");
    statisticsbtn->setFixedSize(110*scaleX, 45*scaleY);

    QVBoxLayout *bpAvgLayout = new QVBoxLayout;
    bpAvgLayout->addWidget(bpVal, 0, Qt::AlignCenter);
    bpAvgLayout->addWidget(avgVal, 0, Qt::AlignCenter);

    QVBoxLayout *hrStatLayout = new QVBoxLayout;
    hrStatLayout->addWidget(hrVal, 0, Qt::AlignCenter);
    hrStatLayout->addWidget(statisticsbtn, 0, Qt::AlignCenter);

    dataLayout->addLayout(bpAvgLayout, Qt::AlignVCenter);
    dataLayout->addLayout(hrStatLayout);

    // 右一：操作按钮
    QWidget *btnWidget = new QWidget;
    btnWidget->setFixedHeight(120*scaleY);
    btnWidget->setStyleSheet(DARK_CARD_STYLE);
    QHBoxLayout *btnLayout = new QHBoxLayout(btnWidget);

    startBtn = new QPushButton(tr("开始测量"));
//    inputCO = new QPushButton(tr("输入心输出量"));
    inputCO = new QPushButton(tr("导出本次数据"));
    inputRHC = new QPushButton(tr("输入RHC"));
    statBtn = new QPushButton(tr("审计界面"));

    startBtn->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #66FF66, stop:1 #33CC33);
            color: white; font-size: 16px; font-weight: bold;
            border: 2px solid #228822; border-radius: 10px; padding: 2px 5px;
        }
        QPushButton:pressed { background: #2EA836; padding-top: 12px; padding-bottom: 8px; }
    )");
    QString secBtnStyle = R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 rgba(180,180,255,40), stop:1 rgba(120,130,200,30));
            color: white; font-size: 14px; border: 1px solid rgba(255,255,255,40);
            border-radius: 8px; padding: 2px 5px;
        }
        QPushButton:pressed { background: rgba(100,120,200,50); }
    )";
    inputCO->setStyleSheet(secBtnStyle);
    inputRHC->setStyleSheet(secBtnStyle);
    statBtn->setStyleSheet(secBtnStyle);

    startBtn->setFixedSize(150*scaleX, 45*scaleY);
    inputCO->setFixedSize(150*scaleX, 45*scaleY);
    inputRHC->setFixedSize(150*scaleX, 45*scaleY);
    statBtn->setFixedSize(150*scaleX, 45*scaleY);

    QVBoxLayout *leftBtns = new QVBoxLayout;
    leftBtns->addWidget(inputCO);
    leftBtns->addSpacing(10*scaleY);
    leftBtns->addWidget(inputRHC);

    QVBoxLayout *rightBtns = new QVBoxLayout;
    rightBtns->addWidget(startBtn);
    rightBtns->addSpacing(10*scaleY);
    rightBtns->addWidget(statBtn);

    btnLayout->addLayout(leftBtns);
    btnLayout->addLayout(rightBtns);

    // 组装底部布局
    bottomLayout->addSpacing(15*scaleX);
    bottomLayout->addWidget(signalWidget);
    bottomLayout->addSpacing(15*scaleX);
    bottomLayout->addWidget(posWidget);
    bottomLayout->addSpacing(15*scaleX);
    bottomLayout->addWidget(dataWidget);
    bottomLayout->addSpacing(15*scaleX);
    bottomLayout->addWidget(btnWidget);
    bottomLayout->addSpacing(15*scaleX);

    mainLayout->addWidget(topWidget);
    mainLayout->addWidget(middleWidget);
    mainLayout->addLayout(bottomLayout);

    // 6. 信号连接
    connect(inputCO, &QPushButton::clicked, this, &ImplantMonitor::openCOClicked);
    connect(inputRHC, &QPushButton::clicked, this, &ImplantMonitor::openRHCClicked);
    connect(startBtn, &QPushButton::clicked, this, &ImplantMonitor::startMeasurement);
    connect(statBtn, &QPushButton::clicked, this, &ImplantMonitor::openReviewClicked);
    connect(statisticsbtn, &QPushButton::clicked, this, &ImplantMonitor::onReadoutButtonClicked);

//    //测试代码-----------------
//    connect(g_DeviceAcquisitionWorker,
//            &DeviceAcquisitionWorker::acquisitionStoppedData,
//            this,
//            &ImplantMonitor::onAcquisitionStoppedData,
//            Qt::QueuedConnection);
}

ImplantMonitor::~ImplantMonitor() {}

// 核心逻辑：主动控制测量流程
void ImplantMonitor::startMeasurement() {
    if (isMeasuring) {
        MedicalLogger::instance()->writeLog(
               "Measurement",
               MedicalLogger::LOG_WARN,
               "Measurement already in progress, duplicate click ignored",
               " ",
               m_serial
           );
        return;
    }
    isMeasuring = true;

    m_plotPoints.clear();

//    plot->clearSimpleData(); // 清空plot显示
    plot->replot();

    bpVal->setText(tr("血压\n0.00/0.00"));
    avgVal->setText(tr("平均\n0.00"));
    hrVal->setText(tr("心率\n0.00"));
    startBtn->setText(tr("测量中..."));
    startBtn->setEnabled(false);

    // 启动线程（队列调用，确保线程安全）
    initWorkersAndConnections();
}

void ImplantMonitor::initWorkersAndConnections() {
    // 前置检查：全局工作对象必须有效
    if (!g_MeasurementDataProcessor || !g_DeviceAcquisitionWorker) {
        MedicalLogger::instance()->writeLog(
            "Threading",
            MedicalLogger::LOG_ERROR,
            "Global worker object not initialized, thread startup aborted",
            " ",
            "System"
        );
        // 回滚前端状态
        isMeasuring = false;
        startBtn->setText(tr("开始测量"));
        startBtn->setEnabled(true);
        return;
    }

    // 阶段1：优先完成所有信号连接（确保数据链路就绪）
    // 1.1 数据处理器→前端波形更新（唯一连接，避免重复）
    static bool isDataParsedConnected = false;
    if (!isDataParsedConnected) {
        bool ok1 = connect(g_MeasurementDataProcessor, &MeasurementDataProcessor::dataParsed,
                                   this, &ImplantMonitor::updateWaveform, Qt::UniqueConnection);
//        qDebug() << "[初始化] dataParsed→updateWaveform 连接结果：" << ok1;
        isDataParsedConnected = ok1;  // 仅当连接成功才标记
        if (!ok1) {
//            qCritical() << "[初始化] 前端波形更新连接失败！";
            return;
        }
    }

    // 1.2 接收线程→数据处理器（核心数据链路，唯一连接）
    static bool isMag5DataConnected = false;
    if (!isMag5DataConnected) {
        bool ok2 = connect(
                    g_DeviceAcquisitionWorker,
                    &DeviceAcquisitionWorker::mag5DataReceived,
                    g_MeasurementDataProcessor,
                    &MeasurementDataProcessor::parseData,
                    Qt::QueuedConnection // 跨线程
                );
//        qDebug() << "[初始化] mag5DataReceived→parseData 连接结果：" << ok2;
        isMag5DataConnected = ok2;  // 仅当连接成功才标记
        if (!ok2) {
//            qCritical() << "[初始化] 核心数据处理链路连接失败！";
            return;
        }
    }

    // 阶段2：启动数据处理器（使用startMeasurement中初始化的时间基准）
    bool invokeOk = QMetaObject::invokeMethod(
        g_MeasurementDataProcessor, "setMeasuring",
        Qt::QueuedConnection,
        Q_ARG(bool, true)  // 复用前端已初始化的时间
    );
//    qDebug() << "[初始化] 启动数据处理器（setMeasuring）：" << invokeOk;
    if (!invokeOk) {
//        qCritical() << "[初始化] 数据处理器启动失败！";
        // 回滚状态
        isMeasuring = false;
        startBtn->setText(tr("开始测量"));
        startBtn->setEnabled(true);
        return;
    }

    // 阶段3：启动UDP接收（先停止旧任务，避免资源冲突）
    // 3.1 停止可能存在的旧接收任务
    QMetaObject::invokeMethod(
        g_DeviceAcquisitionWorker, "stopAcquisition",  // 假设存在停止方法
        Qt::QueuedConnection
    );

    // 3.2 启动新的接收任务
    bool startOk = QMetaObject::invokeMethod(
        g_DeviceAcquisitionWorker, "startAcquisition",
        Qt::QueuedConnection
    );
//    qDebug() << "[初始化] 启动UDP接收：" << startOk;
    if (!startOk) {
//        qCritical() << "[初始化] UDP接收启动失败！";
        // 回滚状态
        QMetaObject::invokeMethod(g_MeasurementDataProcessor, "setMeasuring", Qt::QueuedConnection, Q_ARG(bool, false));
        isMeasuring = false;
        startBtn->setText(tr("开始测量"));
        startBtn->setEnabled(true);
        return;
    }
//    connect(g_MeasurementDataProcessor, &MeasurementDataProcessor::measureFinished,
//            g_DeviceAcquisitionWorker,   &DeviceAcquisitionWorker::stopAcquisition,
//            Qt::QueuedConnection);
    connect(g_MeasurementDataProcessor, &MeasurementDataProcessor::measureFinished,
            this, &ImplantMonitor::stopMeasurement,
            Qt::QueuedConnection);
}

void ImplantMonitor::updateWaveform(const MeasurementData &data) {
    if (!isMeasuring || data.points.isEmpty()) return;

    for (const QPointF &pt : data.points) {
        m_plotPoints.append(pt); // 只存一份数据
    }
    plot->setLiveMode(true, /*windowSec=*/8.0); // 例如 8 秒窗口

    plot->setSimpleData(m_plotPoints);

    // 更新数值显示
    if (bpVal) {
        bpVal->setText(tr("血压\n%1/%2").arg(data.sensorSystolic).arg(data.sensorDiastolic));
    }
    if (avgVal) {
        avgVal->setText(tr("平均\n%1").arg(data.sensorAvg));
    }
    if (hrVal) {
        hrVal->setText(tr("心率\n%1").arg(data.heartRate));
    }
}

void ImplantMonitor::stopMeasurement() {
    MedicalLogger::instance()->writeLog(
        "Measurement",                        // 模块：与测量相关
        MedicalLogger::LOG_INFO,              // 日志等级：信息
        "Measurement stopped",                 // 日志内容：停止测量
        " ",                    // 操作员 ID（未登录时用占位符）
        "UI"                                   // 传感器/子系统 ID（此处为 UI 操作）
    );
    if (!isMeasuring) return;
    isMeasuring = false;

    // 恢复按钮状态
    startBtn->setText(tr("开始测量"));
    startBtn->setEnabled(true);

    // 停 UDP 采集（跨线程排队）
    QMetaObject::invokeMethod(g_DeviceAcquisitionWorker, "stopAcquisition",
                              Qt::QueuedConnection);

//    // 停数据处理（跨线程排队）
//    QMetaObject::invokeMethod(g_MeasurementDataProcessor, "stopMeasuring",
//                              Qt::QueuedConnection);

    plot->setLiveMode(false);      // 关闭实时滚动

    plot->showFullSimpleWaveform();

//     保存提示弹窗
    CustomMessageBox dlg(this, tr("提示"), tr("数据保存成功"),
                         {tr("确 认"), tr("取 消")}, 350 * scaleX);
    int result = dlg.exec();
    if (result == 1) { // 点击“确认”
        openSaveConfirm();
        MedicalLogger::instance()->writeLog(
            "DataSave",                    // 模块名：与数据保存相关
            MedicalLogger::LOG_INFO,        // 日志等级：信息
            "User confirmed the data save", // 日志内容：用户确认保存数据
            "UnknownOperator",              // 操作员 ID（未登录时使用占位符）
            "UI"                            // 来源：UI 操作
        );
    }
}

void ImplantMonitor::openSaveConfirm() {
    MeasurementData data;
    QDateTime now = QDateTime::currentDateTime();
    data.timestamp = now.toString("yyyy-MM-dd HH:mm:ss");
    data.sensorId = idLabel->text();

    // 解析血压（格式："血压\n120.00/80.00"）
    if (bpVal) {
        QStringList bpParts = bpVal->text().split("\n")[1].split("/");
        if (bpParts.size() == 2) {
            data.sensorSystolic = bpParts[0].trimmed();
            data.sensorDiastolic = bpParts[1].trimmed();
        }
    }

    // 解析平均压（格式："平均\n93.33"）
    if (avgVal) {
        data.sensorAvg = avgVal->text().split("\n")[1].trimmed();
    }

    // 解析心率（格式："心率\n75.00"）
    if (hrVal) {
        data.heartRate = hrVal->text().split("\n")[1].trimmed();
    }

    // 波形数据与序号
    data.points = m_plotPoints;
    data.order = measurementList.size() + 1;
    measurementList.append(data);

    // 提示 + 通知表格更新
    CustomMessageBox successDlg(
        this,
        tr("提示"),
        tr("数据保存成功"),
        {tr("确 认")},
        350 * scaleX
    );
    successDlg.exec();
    emit dataListUpdated(measurementList);
}

void ImplantMonitor::openCOClicked() {
//    QWidget *overlay = new QWidget(this);
//    overlay->setStyleSheet("background: rgba(0,0,0,100);");
//    overlay->setAttribute(Qt::WA_TransparentForMouseEvents, false);
//    overlay->show();
//    overlay->raise();

//    QGraphicsBlurEffect *blur = new QGraphicsBlurEffect;
//    blur->setBlurRadius(20);
//    setGraphicsEffect(blur);

//    CardiacOutputDialog *dlg = new CardiacOutputDialog("15", "0", this);
//    dlg->setAttribute(Qt::WA_DeleteOnClose);
//    connect(dlg, &QDialog::finished, this, [=]() {
//        setGraphicsEffect(nullptr);
//        overlay->close();
//        overlay->deleteLater();
//    });
//    dlg->show();
    exportCurrentData();
}

void ImplantMonitor::openRHCClicked() {
    MedicalLogger::instance()->writeLog(
        "RHCInputDialog",                      // 模块：与测量相关
        MedicalLogger::LOG_INFO,            // 日志等级：信息
        "RHC Input dialog opened",          // 日志内容：打开 RHC 输入对话框
        " ",                  // 操作员 ID（未登录时使用占位符）
        "UI"                                // 来源：UI 操作
    );
    QWidget *overlay = new QWidget(this);
    overlay->setStyleSheet("background: rgba(0,0,0,100);");
    overlay->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    overlay->show();
    overlay->raise();

    QGraphicsBlurEffect *blur = new QGraphicsBlurEffect;
    blur->setBlurRadius(20);
    setGraphicsEffect(blur);

    RHCInputDialog *dlg = new RHCInputDialog(this);
    connect(dlg, &QDialog::finished, this, [=]() {
        setGraphicsEffect(nullptr);
        overlay->close();
        overlay->deleteLater();
    });
    dlg->show();
}

void ImplantMonitor::onReadoutButtonClicked() {
    MedicalLogger::instance()->writeLog(
        "ReadoutRecordDialog",                      // 模块名：与测量相关
        MedicalLogger::LOG_INFO,            // 日志等级：信息
        "Readout button clicked",           // 日志内容：用户点击了读数按钮
        " ",                  // 操作员 ID（未登录时使用占位符）
        "UI"                                // 来源：UI 操作
    );
    if (!readoutdialog) {
        readoutdialog = new ReadoutRecordDialog(this);
        readoutdialog->setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
        connect(this, &ImplantMonitor::dataListUpdated, readoutdialog, &ReadoutRecordDialog::populateData);
        connect(readoutdialog, &ReadoutRecordDialog::rowDeleted, this, &ImplantMonitor::onRowDeleted);
        connect(readoutdialog, &ReadoutRecordDialog::onRefreshButtonClicked, this, [this]() {
            measurementList.clear();
            emit returnImplantationsite();
            close();
        });
    }
    emit dataListUpdated(measurementList);
    readoutdialog->show();
}

void ImplantMonitor::onRowDeleted(int row) {
    if (row < 0 || row >= measurementList.size()) return;

    measurementList.removeAt(row);
    for (int i = 0; i < measurementList.size(); ++i) {
        measurementList[i].order = i + 1;
    }
    emit dataListUpdated(measurementList);
    MedicalLogger::instance()->writeLog(
        "ImplantMonitor",                      // 模块名：与测量相关
        MedicalLogger::LOG_INFO,            // 日志等级：信息
        QString("Row %1 deleted").arg(row), // 日志内容：删除了第几行
        " ",                  // 操作员 ID（未登录时使用占位符）
        "UI"                                // 来源：UI 操作
    );
}

void ImplantMonitor::openReviewClicked() {

    if (!reviewwidget) {
        reviewwidget = new ReviewWidget(nullptr, m_serial);
        connect(reviewwidget, &ReviewWidget::returnToImplantmonitor, this, [this]() {
                reviewwidget->setDataList(measurementList);
                this->show();
            QTimer::singleShot(300, this, [this]() {
                reviewwidget->hide();
                reviewwidget->close();
            });
        });
    }
    reviewwidget->setDataList(measurementList);
    reviewwidget->setFixedSize(1024*scaleX, 600*scaleY);
    reviewwidget->show();
    MedicalLogger::instance()->writeLog(
        "ImplantMonitor",                      // 模块：与测量相关
        MedicalLogger::LOG_INFO,            // 日志等级：信息
        "Review window opened",             // 日志内容：打开回顾界面
        " ",                  // 操作员 ID（未登录时使用占位符）
        "UI"                                // 来源：UI 操作
    );
    QTimer::singleShot(200, this, [this]() {
        this->hide();
    });
}

void ImplantMonitor::OpenSettingsRequested() {
    SettingsWidget *dlg = new SettingsWidget();
    dlg->setWindowFlags(Qt::Dialog);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    connect(dlg, &SettingsWidget::signalStrengthChanged, this, [this](int v) {
        if (progressBar) progressBar->setThreshold(v);
        QSettings s("MyCompany", "MyApp");
        s.setValue("system/signalStrength", v);
        s.sync();
    });
    dlg->show();
    MedicalLogger::instance()->writeLog(
        "Settings",                         // 模块：与设置相关
        MedicalLogger::LOG_INFO,            // 日志等级：信息
        "Settings window opened",           // 日志内容：打开设置界面
        "UnknownOperator",                  // 操作员 ID（未登录时使用占位符）
        "UI"                                // 来源：UI 操作
    );
}

void ImplantMonitor::changeEvent(QEvent *event) {
    QWidget::changeEvent(event);
    if (event->type() == QEvent::LanguageChange) {
        titleLabel->setText(tr("新植入物"));
        statisticsbtn->setText(tr("读数记录"));
        startBtn->setText(tr("开始测量"));
//        inputCO->setText(tr("输入心输出量"));
        inputCO->setText(tr("导出本次数据"));
        inputRHC->setText(tr("输入RHC"));
        statBtn->setText(tr("审计界面"));
    }
}

void ImplantMonitor::exportCurrentData() {
    if (m_plotPoints.isEmpty()) {
        QMessageBox::information(this, tr("提示"), tr("当前无测量数据可导出"));
        return;
    }

    // 1. 生成默认文件名（带时间戳，避免重复）
    QDateTime now = QDateTime::currentDateTime();
    QString defaultName = QString("waveform_%1.csv")
                          .arg(now.toString("yyyyMMdd_hhmmss"));

    // 2. 弹出保存对话框，让用户选择路径
    QString filePath = QFileDialog::getSaveFileName(
        this,
        tr("导出波形数据"),
        defaultName,
        tr("CSV文件 (*.csv)")  // 过滤仅显示CSV文件
    );

    if (filePath.isEmpty()) { // 用户取消操作
        qDebug() << "[导出数据] 用户取消保存";
        return;
    }

    // 3. 打开文件（处理打开失败的情况）
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(
            this,
            tr("保存失败"),
            tr("无法打开文件：%1").arg(file.errorString())
        );
        qCritical() << "[导出数据] 文件打开失败：" << file.errorString();
        return;
    }

    // 4. 写入CSV内容（表头 + 数据行）
    QTextStream out(&file);
    out.setRealNumberPrecision(2); // 统一浮点数精度（2位小数）

    // 表头（时间单位：秒，压力单位：mmHg）
    out << tr("时间(s),压力(mmHg)\n");

    // 遍历数据点，逐行写入
    for (const QPointF& pt : m_plotPoints) {
        out << QString("%1,%2\n")
               .arg(pt.x(), 0, 'f', 2)  // 时间：保留2位小数
               .arg(pt.y(), 0, 'f', 1); // 压力：保留1位小数
    }

    // 5. 关闭文件（显式关闭，确保数据落盘）
    file.close();

    // 6. 提示用户保存成功
    QMessageBox::information(
        this,
        tr("导出成功"),
        tr("波形数据已保存至：\n%1").arg(filePath)
    );
    qDebug() << "[导出数据] 成功保存至：" << filePath;
}
