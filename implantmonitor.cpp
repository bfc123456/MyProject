#include "implantmonitor.h"

#include <QScreen>
#include <QGuiApplication>
#include <QSettings>
#include <QGraphicsBlurEffect>
#include <QDebug>
#include "databasemanager.h"
#include "deviceacquisitionworker.h"
#include "custommessagebox.h"
#include "readoutrecorddialog.h"
#include "bluroverlayguard.h"
#include "settingswidget.h"
#include "modernwaveplot.h"
#include "circularprogressbar.h"
#include "cardiacoutputdialog.h"
#include "rhcinputdialog.h"

// 提取重复样式表，统一维护
const QString DARK_CARD_STYLE = R"( QWidget {
                                background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 rgba(25, 50, 75, 0.9), stop:1 rgba(10, 20, 30, 0.85));
                                border-radius: 10px; border: 1px solid rgba(255, 255, 255, 0.08);
                                } )";
ImplantMonitor::ImplantMonitor(QWidget *parent, const QString &sensorId) : FramelessWindow(parent), m_strSensorId(sensorId) {

    // 1. 屏幕缩放计算
    QScreen *screen = QGuiApplication::primaryScreen();
    int screenWidth = screen->geometry().width();
    int screenHeight = screen->geometry().height();
    m_fScaleX = (float)screenWidth / 1024;
    m_fScaleY = (float)screenHeight / 600;
    setFixedSize(1024*m_fScaleX, 600*m_fScaleY);
    setObjectName("Implantonitor");
    setStyleSheet(R"( QWidget#Implantonitor {
                  background-color: qlineargradient(x1:0, y1:1, x2:1, y2:0, stop:0 rgba(6, 15, 30, 255), stop:0.5 rgba(18, 35, 65, 255), stop:1 rgba(30, 60, 100, 255));
                  } )");
    // 2. 主布局初始化
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(30*m_fScaleX, 15*m_fScaleY, 30*m_fScaleX, 15*m_fScaleY);
    // 3. 顶部栏（标题 + 设置按钮）
    QWidget *topWidget = new QWidget(this);
    topWidget->setObjectName("TopBar");
    topWidget->setStyleSheet(R"(
                             QWidget#TopBar {
                             background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 rgba(25, 50, 75, 0.9), stop:1 rgba(10, 20, 30, 0.85));
                             border-radius: 10px;
                             border: 1px solid rgba(255, 255, 255, 0.08);
                             }
                             QLabel {
                             color: white;
                             font-weight: bold;
                             font-size: 16px;
                             } )");
    topWidget->setFixedHeight(50*m_fScaleY);
    m_pTitleLbl = new QLabel(tr(" --- 新植入物 --- "));
    m_pIdLbl = new QLabel(sensorId);
    m_pTitleLbl->setStyleSheet("background: transparent;");
    m_pIdLbl->setStyleSheet("background: transparent;");
    m_pTitleLbl->setFixedWidth(120*m_fScaleX);
    m_pIdLbl->setFixedWidth(120*m_fScaleX);
    m_pTitleLbl->setAlignment(Qt::AlignCenter);
    m_pIdLbl->setAlignment(Qt::AlignCenter);
    QPushButton *settingsBtn = new QPushButton(this);
    settingsBtn->setIcon(QIcon(":/image/icons8-shezhi.png"));
    settingsBtn->setIconSize(QSize(24*m_fScaleX, 24*m_fScaleY));
    settingsBtn->setStyleSheet("border: none; background: transparent; border-radius: 20px;");
    connect(settingsBtn, &QPushButton::clicked, this, &ImplantMonitor::SlotOpenSettingsRequested);
    QHBoxLayout *topLayout = new QHBoxLayout(topWidget);
    topLayout->addWidget(m_pTitleLbl, 0, Qt::AlignLeft);
    topLayout->addWidget(m_pIdLbl, 1, Qt::AlignCenter);
    topLayout->addSpacing(96*m_fScaleX);
    topLayout->addWidget(settingsBtn, 0, Qt::AlignRight);
    // 4. 中间波形区
    QWidget *middleWidget = new QWidget(this);
    middleWidget->setFixedHeight(350*m_fScaleY);
    middleWidget->setStyleSheet(DARK_CARD_STYLE);
    m_pPlot = new ModernWavePlot(this);
    m_pPlot->setStyleSheet("background: transparent; border: none; color:white;");
    m_pPlot->setLineColor(QColor(100, 180, 255));
    m_pPlot->setFillColor(QColor(40, 120, 200, 30), -1);
    m_pPlot->setMinimumHeight(200*m_fScaleY);
    // QwtText xTitle("Time (ms)"), yTitle("Frequency (HZ)");
    // m_pPlot->axisWidget(QwtPlot::xBottom)->setTitle(xTitle);
    // m_pPlot->axisWidget(QwtPlot::yLeft)->setTitle(yTitle);
    //控制栏容器
    QWidget *controlBarwidget = new QWidget(middleWidget);
    controlBarwidget->setStyleSheet("background: transparent; color: white;");
    QHBoxLayout *controlBarLayout = new QHBoxLayout(controlBarwidget);
    //边距按m_fScaleX缩放（原10->10*m_fScaleX，原5->5*scale*Y）
    controlBarLayout->setContentsMargins(qRound(10*m_fScaleX),qRound(5*m_fScaleX),qRound(10*m_fScaleX),qRound(5*m_fScaleX));
    controlBarLayout->setSpacing(qRound(15*m_fScaleX));
    controlBarLayout->setSpacing(qRound(15*m_fScaleX));
    //左侧伸延空间（确保居中）
    controlBarLayout->addStretch();
    //采样率输入方式（数值手动输入，单位下拉选择）
    m_pSampleRateLabel = new QLabel(tr("采样率： "),controlBarwidget);
    QFont sampleFont = m_pSampleRateLabel->font();
    //保持原字体缩放
    sampleFont.setBold(true);
    m_pSampleRateLabel->setFont(sampleFont);
    //数值输入框（手动输入）
    m_pSampleRateEdit = new QLineEdit("8287",controlBarwidget);
    //默认值为1
    m_pSampleRateEdit->setFixedWidth(qRound(30*m_fScaleX));
    //输入框宽度，适配数值输入
    m_pSampleRateEdit->setAlignment(Qt::AlignCenter);//文字居中
    m_pSampleRateEdit->setFont(sampleFont); //单位选择框
    m_pSampleRateUnitCombo = new QComboBox(controlBarwidget);
    m_pSampleRateUnitCombo->addItems({"KHZ","MHZ"}); //常用频率单位
    m_pSampleRateUnitCombo->setCurrentIndex(1); //默认选择HZ
    m_pSampleRateUnitCombo->setEnabled(false); //禁用单位修改
    m_pSampleRateUnitCombo->setMinimumWidth(qRound(30*m_fScaleX)); //单位下拉框宽度
    m_pSampleRateUnitCombo->setFont(sampleFont); //添加到布局（标签+数值输入+单位选择）
    controlBarLayout->addWidget(m_pSampleRateLabel , Qt::AlignVCenter);
    controlBarLayout->addWidget(m_pSampleRateEdit , Qt::AlignVCenter);
    controlBarLayout->addWidget(m_pSampleRateUnitCombo , Qt::AlignVCenter); //分割线1
    controlBarLayout->addSpacing(qRound(5*m_fScaleX));
    QLabel *separator1 = new QLabel("|",controlBarwidget);
    separator1->setStyleSheet("color: #666;");
    separator1->setFont(sampleFont);
    controlBarLayout->addWidget(separator1 , Qt::AlignVCenter);
    controlBarLayout->addSpacing(qRound(5 * m_fScaleX)); //过滤范围输入
    m_pFilterRangeLabel = new QLabel(tr("过滤范围： "),controlBarwidget);
    m_pFilterRangeLabel->setFont(sampleFont); //添加“启动过滤”复选框
    m_pFilterEnabledCheckBox = new QCheckBox(tr("启动"),controlBarwidget);
    m_pFilterEnabledCheckBox->setFont(sampleFont);
    m_pFilterEnabledCheckBox->setChecked(false);//默认不启用过滤
    m_pRangeStartEdit = new QLineEdit("0",controlBarwidget);
    m_pRangeStartEdit->setFixedWidth(qRound(30*m_fScaleX));
    m_pRangeStartEdit->setAlignment(Qt::AlignCenter);
    m_pRangeStartEdit->setFont(sampleFont);
    m_pRangeStartEdit->setEnabled(false);//默认禁用输入框
    QLabel *m_rangeConnector = new QLabel("-",controlBarwidget);
    m_rangeConnector->setFont(sampleFont);
    m_pRangeEndEdit = new QLineEdit("0",controlBarwidget);
    m_pRangeEndEdit->setFixedWidth(qRound(30*m_fScaleX));
    m_pRangeEndEdit->setAlignment(Qt::AlignCenter);
    m_pRangeEndEdit->setFont(sampleFont);
    m_pRangeEndEdit->setEnabled(false);//默认是禁止输入的
    m_pFilterUnitCombo = new QComboBox(controlBarwidget);
    m_pFilterUnitCombo->addItems({"KHZ","MHZ"});
    m_pFilterUnitCombo->setCurrentIndex(1);//默认hz
    m_pFilterUnitCombo->setMinimumWidth(qRound(30*m_fScaleX));
    m_pFilterUnitCombo->setFont(sampleFont);
    m_pFilterUnitCombo->setEnabled(false); //勾选复选框时启用输入控件，取消勾选时禁用
    connect(m_pFilterEnabledCheckBox,&QCheckBox::toggled,[=](bool checked){
        m_pRangeStartEdit->setEnabled(checked);
        m_pRangeEndEdit->setEnabled(checked);
        m_pFilterUnitCombo->setEnabled(checked);
    });
    //设置布局
    controlBarLayout->addWidget(m_pFilterRangeLabel , Qt::AlignVCenter);
    controlBarLayout->addWidget(m_pFilterEnabledCheckBox , Qt::AlignVCenter);
    controlBarLayout->addWidget(m_pRangeStartEdit , Qt::AlignVCenter);
    controlBarLayout->addWidget(m_rangeConnector , Qt::AlignVCenter);
    controlBarLayout->addWidget(m_pRangeEndEdit , Qt::AlignVCenter);
    controlBarLayout->addWidget(m_pFilterUnitCombo , Qt::AlignVCenter); //分割线2
    controlBarLayout->addSpacing(qRound(5*m_fScaleX));
    QLabel *separator2 = new QLabel("|",controlBarwidget);
    separator2->setStyleSheet("color: #666;");
    separator2->setFont(sampleFont);
    controlBarLayout->addWidget(separator2 , Qt::AlignVCenter);
    controlBarLayout->addSpacing(qRound(5 * m_fScaleX)); //x轴单位选择
    m_pXUnitLabel = new QLabel(tr("X轴单位："),controlBarwidget);
    m_pXUnitLabel->setFont(sampleFont);
    QComboBox *xUnitCombo = new QComboBox(controlBarwidget);
    xUnitCombo->addItems({"ms","s"}); //x轴时间单位选项
    connect(xUnitCombo,&QComboBox::currentTextChanged,m_pPlot,&ModernWavePlot::onXUnitChanged);
    xUnitCombo->setCurrentIndex(0); //
    xUnitCombo->setMinimumWidth(qRound(60*m_fScaleX));//下拉框宽度
    xUnitCombo->setFont(sampleFont); controlBarLayout->addWidget(m_pXUnitLabel , Qt::AlignVCenter);
    controlBarLayout->addWidget(xUnitCombo , Qt::AlignVCenter); //分割线3
    controlBarLayout->addSpacing(qRound(5*m_fScaleX));
    QLabel *separator3 = new QLabel("|",controlBarwidget);
    separator3->setStyleSheet("color: #666;");
    separator3->setFont(sampleFont);
    controlBarLayout->addWidget(separator3 , Qt::AlignVCenter);
    controlBarLayout->addSpacing(qRound(5 * m_fScaleX)); //y轴单位选择
    m_pYUnitLabel = new QLabel(tr("Y轴单位："),controlBarwidget);
    m_pYUnitLabel->setFont(sampleFont);
    QComboBox *yUnitCombo = new QComboBox(controlBarwidget);
    yUnitCombo->addItems({"KHZ","MHZ"}); //x轴时间单位选项
    connect(yUnitCombo,&QComboBox::currentTextChanged,m_pPlot,&ModernWavePlot::onYUnitChanged);
    yUnitCombo->setCurrentIndex(1); //
    yUnitCombo->setMinimumWidth(qRound(60*m_fScaleX));//下拉框宽度
    yUnitCombo->setFont(sampleFont);
    yUnitCombo->setStyleSheet("QComboBox QAbstractItemView { text-align: center; }");
    controlBarLayout->addWidget(m_pYUnitLabel , Qt::AlignVCenter);
    controlBarLayout->addWidget(yUnitCombo , Qt::AlignVCenter); //右侧伸缩空间
    controlBarLayout->addStretch(); QVBoxLayout *middleLayout = new QVBoxLayout(middleWidget);
    middleLayout->addWidget(m_pPlot); middleLayout->addSpacing(10*m_fScaleY);
    middleLayout->addWidget(controlBarwidget);
    middleLayout->setContentsMargins(50*m_fScaleX, 25*m_fScaleY, 50*m_fScaleX, 25*m_fScaleY);
    // 5. 底部控件区（信号、位置、数据、按钮）
    QHBoxLayout *bottomLayout = new QHBoxLayout();
    // 左一：信号进度
    QWidget *signalWidget = new QWidget;
    signalWidget->setFixedHeight(120*m_fScaleY);
    signalWidget->setStyleSheet(DARK_CARD_STYLE);
    QVBoxLayout *signalLayout = new QVBoxLayout(signalWidget);
    QSettings settings("MyCompany", "MyApp");
    int signalThresh = settings.value("system/signalStrength", 70).toInt();
    m_pProgressBar = new CircularProgressBar(this);
    m_pProgressBar->setFixedSize(65*m_fScaleX, 65*m_fScaleY);
    m_pProgressBar->setThreshold(signalThresh);
    m_pProgressBar->setProgress(90); signalLayout->addWidget(m_pProgressBar, 0, Qt::AlignCenter);
    //左二：植入位置显示
    QWidget *posWidget = new QWidget(parent);
    posWidget->setObjectName("posWidget");
    posWidget->setFixedSize(180*m_fScaleX, 120*m_fScaleY);
    posWidget->setStyleSheet(R"( QWidget#posWidget {
                             border: none; /* 用 border-image 拉伸背景 */
                             border-image: url(:/image/newbody.png);
                             } )");
    //在它上面放一个 QLabel 来显示“L”或“R”
    sideLabel = new QLabel(posWidget);
    sideLabel->setFixedSize(40*m_fScaleX, 40*m_fScaleY);
    sideLabel->setAlignment(Qt::AlignCenter);
    sideLabel->setStyleSheet(R"( QLabel {
                             background-color: rgba(33, 150, 243, 0.85); /* #2196F3 + 85% 不透明度 */
                             color: white;
                             font-size: 18px;
                             border-radius: 6px;
                             } )");
    QString loc = DatabaseManager::instance().getLocationBySensorId(m_strSensorId);
    m_bIsLeftFlag = (loc == "left");
    // qDebug()<<" "<<m_bIsLeftFlag;
    //根据左右来移动到正确位置，比如右侧偏上
    if (m_bIsLeftFlag) { sideLabel->setText(tr("左"));
        QFont font = sideLabel->font(); font.setBold(true); // 设置加粗
        sideLabel->setFont(font); // 左侧居中偏左
        sideLabel->move( 40, 2*(posWidget->height() - sideLabel->height())/3); }
    else {
        sideLabel->setText(tr("右"));
        QFont font = sideLabel->font();
        font.setBold(true); // 设置加粗
        sideLabel->setFont(font); // 右侧居中偏右
        sideLabel->move( (posWidget->width() - sideLabel->width() - 40), 2*(posWidget->height() - sideLabel->height())/3 ); }
    sideLabel->show();
    // 右二：数据显示
    QWidget *dataWidget = new QWidget;
    dataWidget->setFixedSize(260*m_fScaleX, 120*m_fScaleY);
    dataWidget->setStyleSheet(DARK_CARD_STYLE);
    QHBoxLayout *dataLayout = new QHBoxLayout(dataWidget);
    m_pBpValLbl = new QLabel(tr("血压\n0.00/0.00"), this);
    m_pBpValLbl->setFixedSize(110*m_fScaleX,45*m_fScaleY);
    m_pBpValLbl->setAlignment(Qt::AlignCenter);
    m_pAvgValLbl = new QLabel(tr("平均\n0.00"), this);
    m_pAvgValLbl->setFixedSize(110*m_fScaleX,45*m_fScaleY);
    m_pAvgValLbl->setAlignment(Qt::AlignCenter);
    m_pHrValLbl = new QLabel(tr("心率\n0.00"), this);
    m_pHrValLbl->setFixedSize(110*m_fScaleX,45*m_fScaleY);
    m_pHrValLbl->setAlignment(Qt::AlignCenter);
    m_pStatisticsBtn = new QPushButton(tr("读数记录"));
    QString cardLabelStyle = R"( QLabel {
                             background-color: qlineargradient( x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 rgba(255, 255, 255, 20), stop: 1 rgba(255, 255, 255, 10) );
                             color: white;
                             font-size: 13px;
                             font-weight: bold;
                             border: 1px solid rgba(255, 255, 255, 50);
                             border-radius: 10px; padding: 2px;
                             } )";
    m_pBpValLbl->setStyleSheet(cardLabelStyle);
    m_pAvgValLbl->setStyleSheet(cardLabelStyle);
    m_pHrValLbl->setStyleSheet(cardLabelStyle);
    m_pStatisticsBtn->setStyleSheet(R"(
                                    QPushButton {
                                    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 rgba(180,180,255,40), stop:1 rgba(120,130,200,30));
                                    color: white;
                                    font-size: 14px;
                                    border: 1px solid rgba(255,255,255,40);
                                    border-radius: 8px;
                                    padding: 2px 5px; text-align: center; }
                                    QPushButton:pressed {
                                    background: rgba(100,120,200,50);
                                    } )");
    m_pStatisticsBtn->setFixedSize(110*m_fScaleX, 45*m_fScaleY);
    QVBoxLayout *bpAvgLayout = new QVBoxLayout;
    bpAvgLayout->addWidget(m_pBpValLbl, Qt::AlignVCenter);
    bpAvgLayout->addWidget(m_pAvgValLbl, Qt::AlignVCenter);
    QVBoxLayout *hrStatLayout = new QVBoxLayout;
    hrStatLayout->addWidget(m_pHrValLbl, Qt::AlignVCenter);
    hrStatLayout->addWidget(m_pStatisticsBtn, Qt::AlignVCenter);
    dataLayout->addLayout(bpAvgLayout, Qt::AlignVCenter);
    dataLayout->addLayout(hrStatLayout);
    // 右一：操作按钮
    QWidget *btnWidget = new QWidget; btnWidget->setFixedHeight(120*m_fScaleY);
    btnWidget->setStyleSheet(DARK_CARD_STYLE);
    QHBoxLayout *btnLayout = new QHBoxLayout(btnWidget);
    m_pStartBtn = new QPushButton(tr("开始测量"));
    m_pInputCoBtn = new QPushButton(tr("输入心输出量"));
    m_pInputRhcBtn = new QPushButton(tr("输入RHC"));
    m_pExportBtn = new QPushButton(tr("导出数据"));
    QString secBtnStyle = R"( QPushButton {
                          background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 rgba(180,180,255,40), stop:1 rgba(120,130,200,30));
                          color: white; font-size: 14px;
                          border: 1px solid rgba(255,255,255,40);
                          border-radius: 8px; padding: 2px 5px; }
                          QPushButton:pressed {
                          background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #80E27E, stop:1 #66BB6A); /* 渐变的绿色，更加柔和 */
                          color: white;
                          font-size: 16px;
                          font-weight: bold;
                          border: 2px solid #388E3C; /* 深绿色边框 */
                          border-radius: 10px;
                          padding: 8px 15px; } )";
    m_pStartBtn->setStyleSheet(secBtnStyle);
    m_pInputCoBtn->setStyleSheet(secBtnStyle);
    m_pInputRhcBtn->setStyleSheet(secBtnStyle);
    m_pExportBtn->setStyleSheet(secBtnStyle);
    m_pStartBtn->setFixedSize(150*m_fScaleX, 45*m_fScaleY);
    m_pInputCoBtn->setFixedSize(150*m_fScaleX, 45*m_fScaleY);
    m_pInputRhcBtn->setFixedSize(150*m_fScaleX, 45*m_fScaleY);
    m_pExportBtn->setFixedSize(150*m_fScaleX, 45*m_fScaleY);
    QVBoxLayout *leftBtns = new QVBoxLayout; leftBtns->addWidget(m_pStartBtn);
    leftBtns->addSpacing(10*m_fScaleY); leftBtns->addWidget(m_pInputCoBtn);
    QVBoxLayout *rightBtns = new QVBoxLayout; rightBtns->addWidget(m_pExportBtn);
    rightBtns->addSpacing(10*m_fScaleY);
    rightBtns->addWidget(m_pInputRhcBtn);
    btnLayout->addLayout(leftBtns);
    btnLayout->addLayout(rightBtns); // 组装底部布局
    bottomLayout->addSpacing(15*m_fScaleX);
    bottomLayout->addWidget(signalWidget);
    bottomLayout->addSpacing(15*m_fScaleX);
    bottomLayout->addWidget(posWidget);
    bottomLayout->addSpacing(15*m_fScaleX);
    bottomLayout->addWidget(dataWidget);
    bottomLayout->addSpacing(15*m_fScaleX);
    bottomLayout->addWidget(btnWidget);
    bottomLayout->addSpacing(15*m_fScaleX);
    mainLayout->addWidget(topWidget);
    mainLayout->addWidget(middleWidget);
    mainLayout->addLayout(bottomLayout);
    // 6. 信号连接
    connect(m_pStartBtn, &QPushButton::clicked, this, &ImplantMonitor::onStartMeasurement);
    connect(m_pInputCoBtn, &QPushButton::clicked, this, &ImplantMonitor::SlotOpenCOClicked);
    connect(m_pInputRhcBtn, &QPushButton::clicked, this, &ImplantMonitor::SlotOpenRHCClicked);
    connect(m_pExportBtn, &QPushButton::clicked, this, &ImplantMonitor::SlotExportCurrentData);
    connect(m_pStatisticsBtn, &QPushButton::clicked, this, &ImplantMonitor::SlotOnReadoutButtonClicked);
    applyIdleUiState();
}

ImplantMonitor::~ImplantMonitor() {}

void ImplantMonitor::setupDataPipelineOnce()
{
    static bool inited = false;
    if (inited) return;
    inited = true;

//    QObject::connect(g_pDeviceAcquisitionWorkerPtr, &DeviceAcquisitionWorker::rawPacketReceived,
//                     g_pMeasurementDataProcessorPtr, &MeasurementDataProcessor::onRawPacketArrived,
//                     Qt::QueuedConnection);

//    QObject::connect(g_pDeviceAcquisitionWorkerPtr, &DeviceAcquisitionWorker::fftPacketReceived,
//                     g_pMeasurementDataProcessorPtr, &MeasurementDataProcessor::onFftPacketArrived,
//                     Qt::QueuedConnection);

    QObject::connect(g_pMeasurementDataProcessorPtr, &MeasurementDataProcessor::waveformUpdated,
                     this, &ImplantMonitor::SlotUpdateWaveformFrame,
                     Qt::QueuedConnection);

    QObject::connect(g_pMeasurementDataProcessorPtr, &MeasurementDataProcessor::measureFinished,
                     this, &ImplantMonitor::onMeasureFinished,
                     Qt::QueuedConnection);

    QObject::connect(g_pDeviceAcquisitionWorkerPtr, &DeviceAcquisitionWorker::acquisitionError,
                     this, &ImplantMonitor::onAcquisitionError,
                     Qt::QueuedConnection);
}

void ImplantMonitor::onStartMeasurement()
{
    if (m_bIsMeasuringFlag) return;

    if (!g_pMeasurementDataProcessorPtr || !g_pDeviceAcquisitionWorkerPtr) {
        applyIdleUiState();
        return;
    }

    setupDataPipelineOnce();
    applyMeasuringUiState();

    QMetaObject::invokeMethod(g_pMeasurementDataProcessorPtr, "requestStart", Qt::QueuedConnection);
    QMetaObject::invokeMethod(g_pDeviceAcquisitionWorkerPtr, "requestStart", Qt::QueuedConnection);
}

void ImplantMonitor::SlotStopMeasurement()
{
    if (!m_bIsMeasuringFlag) return;

    applyIdleUiState();

    QMetaObject::invokeMethod(g_pDeviceAcquisitionWorkerPtr, "requestStop", Qt::QueuedConnection);
    QMetaObject::invokeMethod(g_pMeasurementDataProcessorPtr, "requestStop", Qt::QueuedConnection);

    if (m_pPlot) {
        m_pPlot->setLiveMode(false);
    }
}

void ImplantMonitor::SlotUpdateWaveformFrame(const QVector<QPointF> &frame)
{
    if (!m_bIsMeasuringFlag || frame.isEmpty()) return;

    // 如果你担心内存：可以只保留最近N秒/最近N点
    m_vecPlotPoints += frame;

    m_pPlot->setLiveMode(true, 8.0);
    m_pPlot->setSimpleData(m_vecPlotPoints);
}

void ImplantMonitor::onMeasureFinished(const MeasurementData &result)
{
    // 更新数值
    if (m_pBpValLbl)
        m_pBpValLbl->setText(tr("血压\n%1/%2").arg(result.sensorSystolic).arg(result.sensorDiastolic));
    if (m_pAvgValLbl)
        m_pAvgValLbl->setText(tr("平均\n%1").arg(result.sensorAvg));
    if (m_pHrValLbl)
        m_pHrValLbl->setText(tr("心率\n%1").arg(result.heartRate));

    // 最终波形
    if (!result.points.isEmpty()) {
        m_vecPlotPoints = result.points;
        m_pPlot->setSimpleData(m_vecPlotPoints);
        m_pPlot->showFullSimpleWaveform();
    }

    // 保存确认（沿用你原逻辑）
    CustomMessageBox dlg(this, tr("提示"), tr("数据保存成功"),
                         {tr("确 认"), tr("取 消")}, int(350*m_fScaleX));
    int r = dlg.exec();
    if (r == 1) {
        SlotOpenSaveConfirm();
    }
}

void ImplantMonitor::onAcquisitionError(const QString &msg)
{
    qWarning() << "[UI] acquisition error:" << msg;

    CustomMessageBox dlg(this, tr("错误"), msg, {tr("确 认")}, int(350*m_fScaleX));
    dlg.exec();

    SlotStopMeasurement();
}

void ImplantMonitor::SlotOpenSaveConfirm()
{
    MeasurementData data;
    QDateTime now = QDateTime::currentDateTime();
    data.timestamp = now.toString("yyyy-MM-dd HH:mm:ss");
    data.sensorId = m_pIdLbl ? m_pIdLbl->text() : "";

    // 解析血压
    if (m_pBpValLbl) {
        QStringList parts = m_pBpValLbl->text().split("\n");
        if (parts.size() >= 2) {
            QStringList bp = parts[1].split("/");
            if (bp.size() == 2) {
                data.sensorSystolic = bp[0].trimmed();
                data.sensorDiastolic = bp[1].trimmed();
            }
        }
    }
    if (m_pAvgValLbl) {
        QStringList parts = m_pAvgValLbl->text().split("\n");
        if (parts.size() >= 2) data.sensorAvg = parts[1].trimmed();
    }
    if (m_pHrValLbl) {
        QStringList parts = m_pHrValLbl->text().split("\n");
        if (parts.size() >= 2) data.heartRate = parts[1].trimmed();
    }

    data.points = m_vecPlotPoints;
    data.order = m_lstMeasurements.size() + 1;
    m_lstMeasurements.append(data);

    CustomMessageBox successDlg(this, tr("提示"), tr("数据保存成功"), {tr("确 认")}, int(350*m_fScaleX));
    successDlg.exec();

    // emit SigDataListUpdated(m_lstMeasurements); // 你项目里如果需要就打开
}

void ImplantMonitor::SlotExportCurrentData()
{
    if (!g_pMeasurementDataProcessorPtr) return;

    QQueue<QByteArray> rawPackets;
    QQueue<QByteArray> fftPackets;

    // 跨线程调用，安全
    QMetaObject::invokeMethod(
        g_pMeasurementDataProcessorPtr,
        [&]() {
            g_pMeasurementDataProcessorPtr->takeAllQueues(rawPackets, fftPackets);
        },
        Qt::BlockingQueuedConnection   // 导出时允许阻塞
    );

    if (rawPackets.isEmpty() && fftPackets.isEmpty()) {
        QMessageBox::information(this, tr("提示"), tr("当前无原始数据可导出"));
        return;
    }

    exportQueuesToFiles(rawPackets, fftPackets);
}

void ImplantMonitor::exportQueuesToFiles(const QQueue<QByteArray>& rawQ, const QQueue<QByteArray>& fftQ)
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("选择导出目录"));
    if (dir.isEmpty()) return;

    QFile rawFile(dir + "/raw.bin");
    QFile fftFile(dir + "/fft.bin");

    rawFile.open(QIODevice::WriteOnly);
    fftFile.open(QIODevice::WriteOnly);

    for (const auto& p : rawQ)
        rawFile.write(p);

    for (const auto& p : fftQ)
        fftFile.write(p);

    rawFile.close();
    fftFile.close();

    QMessageBox::information(
        this,
        tr("导出成功"),
        tr("已导出 RAW(%1 包) 和 FFT(%2 包) 数据")
            .arg(rawQ.size())
            .arg(fftQ.size())
    );
}

double ImplantMonitor::GetSamplingRateHz() const
{
    bool ok = false;
    double rateVal = m_pSampleRateEdit ? m_pSampleRateEdit->text().toDouble(&ok) : 0.0;
    if (!ok || rateVal <= 0) rateVal = 1.0;

    QString unit = m_pSampleRateUnitCombo ? m_pSampleRateUnitCombo->currentText().toUpper() : "KHZ";

    // 修复：KHZ->Hz: *1e3，MHZ->Hz:*1e6
    double scale = 1.0;
    if (unit == "KHZ") scale = 1e3;
    else if (unit == "MHZ") scale = 1e6;

    return rateVal * scale;
}

QVector<QPointF> ImplantMonitor::DownsampleData(const QVector<QPointF>& originalData, double targetRateHZ) const
{
    if (originalData.isEmpty() || targetRateHZ <= 0)
        return {};

    double targetInterval = 1.0 / targetRateHZ;
    int step = qRound(targetInterval / ORIGINAL_SAMPLING_INTERVAL);
    if (step < 1) step = 1;

    QVector<QPointF> result;
    for (int i = 0; i < originalData.size(); i += step)
        result.append(originalData[i]);

    return result;
}

bool ImplantMonitor::GetFilterRangeHz(double& minHz, double& maxHz)
{
    if (!m_pRangeStartEdit || !m_pRangeEndEdit || !m_pFilterUnitCombo) return false;

    bool ok1=false, ok2=false;
    double minVal = m_pRangeStartEdit->text().toDouble(&ok1);
    double maxVal = m_pRangeEndEdit->text().toDouble(&ok2);

    if (!ok1 || !ok2) return false;
    if (minVal == 0 && maxVal == 0) return false;
    if (minVal < 0 || maxVal < 0) return false;
    if (minVal > maxVal) return false;

    // 修复：大小写统一
    QString unit = m_pFilterUnitCombo->currentText().toUpper();
    double scale = 1.0;
    if (unit == "KHZ") scale = 1e3;
    else if (unit == "MHZ") scale = 1e6;
    else return false;

    minHz = minVal * scale;
    maxHz = maxVal * scale;
    return true;
}

void ImplantMonitor::applyMeasuringUiState()
{
    m_bIsMeasuringFlag = true;

    m_vecPlotPoints.clear();
    if (m_pPlot) {
        m_pPlot->clearSimpleData();
        m_pPlot->setLiveMode(true, 8.0);
        m_pPlot->replot();
    }

    if (m_pBpValLbl)  m_pBpValLbl->setText(tr("血压\n0.00/0.00"));
    if (m_pAvgValLbl) m_pAvgValLbl->setText(tr("平均\n0.00"));
    if (m_pHrValLbl)  m_pHrValLbl->setText(tr("心率\n0.00"));

    if (m_pStartBtn) {
        m_pStartBtn->setText(tr("测量中..."));
        m_pStartBtn->setEnabled(false);
    }
}

void ImplantMonitor::applyIdleUiState()
{
    m_bIsMeasuringFlag = false;
    if (m_pStartBtn) {
        m_pStartBtn->setText(tr("开始测量"));
        m_pStartBtn->setEnabled(true);
    }
}

void ImplantMonitor::SlotOpenSettingsRequested()
{
    SettingsWidget *dlg = new SettingsWidget(this);
    dlg->setWindowFlags(Qt::Dialog);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->show();
}

void ImplantMonitor::SlotOnReadoutButtonClicked()
{
    // 你原来的读数记录对话框逻辑可以放这里
    if (!m_pReadoutDialog) {
        m_pReadoutDialog = new ReadoutRecordDialog(this);
    }
    m_pReadoutDialog->show();
}

void ImplantMonitor::SlotOnRowDeleted(int row)
{
    if (row < 0 || row >= m_lstMeasurements.size()) return;
    m_lstMeasurements.removeAt(row);
    for (int i = 0; i < m_lstMeasurements.size(); ++i)
        m_lstMeasurements[i].order = i + 1;
}

void ImplantMonitor::changeEvent(QEvent *event)
{
    FramelessWindow::changeEvent(event);
    if (event->type() == QEvent::LanguageChange) {
        if (m_pTitleLbl) m_pTitleLbl->setText(tr(" --- 新植入物 --- "));
        if (m_pStartBtn) m_pStartBtn->setText(tr("开始测量"));
    }
}

void ImplantMonitor::SlotOpenCOClicked() {
    QWidget *overlay = new QWidget(this);
    overlay->setStyleSheet("background: rgba(0,0,0,100);");
    overlay->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    overlay->show();
    overlay->raise();

    QGraphicsBlurEffect *blur = new QGraphicsBlurEffect;
    blur->setBlurRadius(20);
    setGraphicsEffect(blur);

    CardiacOutputDialog *dlg = new CardiacOutputDialog("15", "0", this);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    connect(dlg, &QDialog::finished, this, [=]() {
        setGraphicsEffect(nullptr);
        overlay->close();
        overlay->deleteLater();
    });
    dlg->show();
}

void ImplantMonitor::SlotOpenRHCClicked() {
//    MedicalLogger::instance()->writeLog(
//        "RHCInputDialog",                      // 模块：与测量相关
//        MedicalLogger::LOG_INFO,            // 日志等级：信息
//        "RHC Input dialog opened",          // 日志内容：打开 RHC 输入对话框
//        " ",                  // 操作员 ID（未登录时使用占位符）
//        "UI"                                // 来源：UI 操作
//    );
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
