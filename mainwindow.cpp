#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "historycheck.h"
#include "CustomMessageBox.h"
#include "settingswidget.h"
#include <QApplication>
#include <QStyle>
#include <QScreen>
#include <QRect>
#include <QPainter>
#include <QPen>
#include <QColor>
#include <QByteArray>
#include <QVector>
#include <QTimer>
#include <QRandomGenerator>//模拟量测试
#include <QTime>
#include <QMessageBox>
#include <QDebug>
#include <random> //生成随机数的标注库
#include <qwt_plot_grid.h>
#include <QHeaderView>
#include <atomic>
#include <QTranslator>
#include <QElapsedTimer>
#include <qwt_scale_widget.h>
#include <qwt_plot_canvas.h>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      diastolicValue(new QLabel("---", this)),
      systolicValue(new QLabel("---", this)),
      avgValue(new QLabel("---", this)),
      heartbeatValue(new QLabel("---", this)),
      isMeasuring(false),
      currentTime(0),
      maxPressure(25.0),
      minPressure(6.0),
      serialManager(new SerialPortManager(this))
{
    // 检查是否成功连接数据库
    if (dbManager.connectDatabase()) {
        qDebug() << "Database connected successfully!";
    } else {
        qDebug() << "Failed to connect to database.";
    }

    ui->setupUi(this);

    this->setWindowTitle(tr("主操作界面"));
    this->setFixedSize(1024, 600);  // 固定窗口大小
    this->setStyleSheet("background-color:#000000;");  // **整体背景哑光黑**

    //配置串口信息
    connect(serialManager, &SerialPortManager::dataReceived, this, &MainWindow::onDataReceived);
    connect(serialManager, &SerialPortManager::errorOccurred, this, &MainWindow::onErrorOccurred);

    //配置串口
    serialManager->openPort("COM3", QSerialPort::Baud115200);


    //plot数据设定及框图
    // 初始化数据
    xData.clear();
    yData.clear();

    // 准备数据
    for (int i = 0; i < 100; ++i) {
        xData.push_back(i);
        yData.push_back(0.0); // 初始化为0
    }

    // **计算左右固定宽度**
    int leftWidth = 716;  // 1024 * 0.7
    int rightWidth = 308; // 1024 * 0.3

    // **创建主窗口的中心部件**
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(0);

    // **创建 QSplitter 进行水平分割**
    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setHandleWidth(0);  // 禁止拖拽

    // **左侧区域**
    // **初始化 QwtPlot**
    plot = ui->qwtPlot;
    curve = new QwtPlotCurve();

    // **创建曲线并设置数据**
    curve->setData(new QwtPointSeriesData(generateData()));  // 直接传递数据，减少变量
    curve->setPen(QPen(QColor(0, 255, 0), 2));  // 绿色曲线，增强可见度
    curve->attach(plot);

    // **设置 X/Y 轴标题**
    QwtText xAxisTitle(tr("时间 (s)"));
    QwtText yAxisTitle(tr("压力 (mmHg)"));

    // **设置字体**
    QFont axisFont("黑体", 16, QFont::Bold);
    xAxisTitle.setFont(axisFont);
    yAxisTitle.setFont(axisFont);

    // **设置标题颜色**
    xAxisTitle.setColor(Qt::white);
    yAxisTitle.setColor(Qt::white);

    // **应用标题**
    plot->setAxisTitle(QwtPlot::xBottom, xAxisTitle);
    plot->setAxisTitle(QwtPlot::yLeft, yAxisTitle);

    // **设置坐标轴字体**
    plot->setAxisFont(QwtPlot::xBottom, axisFont);
    plot->setAxisFont(QwtPlot::yLeft, axisFont);

    // **使用 QPalette 设置刻度颜色**
    QPalette axisPalette;
    axisPalette.setColor(QPalette::Text, Qt::white);
    axisPalette.setColor(QPalette::Foreground, Qt::white);  // **确保坐标轴颜色也变白色**
    plot->axisWidget(QwtPlot::xBottom)->setPalette(axisPalette);
    plot->axisWidget(QwtPlot::yLeft)->setPalette(axisPalette);

    // 设置 X 轴（时间范围：0 - 60 秒）
    plot->setAxisScale(QwtPlot::xBottom, 0, 60);

    // 设置 Y 轴（压力范围：0 - 60 mmHg）
    plot->setAxisScale(QwtPlot::yLeft, 0, 60);

    // **获取 X 轴 和 Y 轴的 `QwtScaleWidget`**
    QwtScaleWidget *xAxis = plot->axisWidget(QwtPlot::xBottom);
    QwtScaleWidget *yAxis = plot->axisWidget(QwtPlot::yLeft);

    if (xAxis && yAxis) {
        // **设置坐标轴刻度字体颜色**
        QPalette axisPalette;
        axisPalette.setColor(QPalette::Text, Qt::white);  // 刻度文本颜色
        axisPalette.setColor(QPalette::Foreground, Qt::white);  // 刻度线颜色

        xAxis->setPalette(axisPalette);
        yAxis->setPalette(axisPalette);

        // **设置刻度字体大小**
        QFont tickFont("黑体", 10, QFont::Bold);  // 设置刻度字体大小为 14px
        xAxis->setFont(tickFont);  // **应用刻度字体到 X 轴**
        yAxis->setFont(tickFont);  // **应用刻度字体到 Y 轴**
    }


    // **优化网格**
    QwtPlotGrid *grid = new QwtPlotGrid();
    grid->enableX(false);  // 仅启用 X 轴网格
    grid->enableY(true); // 禁用 Y 轴网格
    grid->setMajorPen(QPen(QColor(150, 150, 150), 1, Qt::DashLine));  // 灰色虚线
    grid->attach(plot);

    // **创建 QFrame 容器**
    QFrame *plotContainer = new QFrame(this);
    plotContainer->setStyleSheet(R"(
        QFrame {
            background-color: 1F1F1F;
            border-radius: 12px;
            border: 2px solid #444444;
            margin-right: 10px;
        }
    )");

    // **优化 QVBoxLayout**
    QVBoxLayout *plotContainerLayout = new QVBoxLayout(plotContainer);
    plotContainerLayout->setContentsMargins(10, 10, 10, 10);  // **增加内边距**
    plotContainerLayout->setSpacing(5); // **调整间距**
    plotContainerLayout->addWidget(plot);
    plotContainer->setLayout(plotContainerLayout);

    // **优化 QwtPlot 样式**
    plot->setStyleSheet("border: none;");  // 只去除边框，不影响背景
    plot->setCanvasBackground(QColor("#000000"));  // 设置黑色背景

    // **确保更新显示**
    plot->replot();

    QFrame *leftFrame = new QFrame();
    leftFrame->setFrameShape(QFrame::NoFrame);
    leftFrame->setFixedWidth(leftWidth);
    leftFrame->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    QVBoxLayout *leftLayout = new QVBoxLayout(leftFrame);
    leftLayout->setContentsMargins(0, 0, 0, 0);

    // **按钮区域**
    QFrame *buttonContainer = new QFrame();
    buttonContainer->setStyleSheet(R"(
        QFrame {
            background-color: #222222;
            border-radius: 12px;
            border: 2px solid #444444;
            margin-right: 10px;
        }
    )");
    QVBoxLayout *buttonContainerLayout = new QVBoxLayout(buttonContainer);
    buttonContainerLayout->setContentsMargins(10, 10, 10, 10);

    QFrame *buttonFrame = new QFrame();
    buttonFrame->setFrameShape(QFrame::NoFrame);
    buttonFrame->setStyleSheet("background-color: transparent; border: none;"); // **确保隐藏边框**
    QHBoxLayout *buttonLayout = new QHBoxLayout(buttonFrame);

    measureButton = new QPushButton(tr("开始量测"), buttonFrame);
    measureButton->setIcon(QIcon(":/image/start.png"));
    uploadButton = new QPushButton(tr("上传数据"), buttonFrame);
    uploadButton->setIcon(QIcon(":/image/updata.png"));
    historyButton = new QPushButton(tr("历史查询"), buttonFrame);
    historyButton->setIcon(QIcon(":/image/history.png"));
    settingsButton = new QPushButton(tr("参数调节"), buttonFrame);
    settingsButton->setIcon(QIcon(":/image/setting.png"));
    settingsButton->setIconSize(QSize(32, 32));  // 调整图标大小

    //创建定时器并初始化
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::onTimerTimeout);


    // 设置定时器初始的状态
        isMeasuring = false;
        currentTime = 0;


    measureButton->setMinimumSize(75, 60);
    uploadButton->setMinimumSize(75, 60);
    historyButton->setMinimumSize(75, 60);
    settingsButton->setMinimumSize(75, 60);

    QString buttonStyle = R"(
        QPushButton {
            background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #757575, stop: 1 #616161);  /* 立体渐变 */
            color: white;  /* 文字颜色 */
            font-size: 18px;
            font-family: "黑体";  /* 设置字体为黑体 */
            font-weight: bold;
            border: 2px solid #666666;  /* 默认边框 */
            border-radius: 8px;
            padding: 8px 16px;
            icon-size: 24px;  /* 调整图标大小 */
        }

        QPushButton:pressed {
            background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #4A4A4A, stop: 1 #383838);  /* 按下时颜色稍深 */
            border: 2px solid white;
        }

        QPushButton:disabled {
            background: #444;  /* 禁用时颜色变暗 */
            color: #888;
            border: 2px solid #555;
        }
    )";





    // 应用样式到每个按钮
    measureButton->setStyleSheet(buttonStyle);
    uploadButton->setStyleSheet(buttonStyle);
    historyButton->setStyleSheet(buttonStyle);
    settingsButton->setStyleSheet(buttonStyle);

    buttonLayout->addWidget(measureButton);
    buttonLayout->addWidget(uploadButton);
    buttonLayout->addWidget(historyButton);
    buttonLayout->addWidget(settingsButton);
    buttonFrame->setLayout(buttonLayout);

    buttonContainerLayout->addWidget(buttonFrame);
    buttonContainer->setLayout(buttonContainerLayout);

    // 设置按钮点击事件
//    connect(measureButton, &QPushButton::clicked, this, &MainWindow::on_pushButtonSend_clicked);//测试串口通信

    connect(measureButton, &QPushButton::clicked, this, &MainWindow::onStartClicked);
    connect(uploadButton, &QPushButton::clicked, this, &MainWindow::onUploadDataClicked);
    connect(historyButton, &QPushButton::clicked, this, &MainWindow::onHistoryButtonClicked);
//    connect(settingsButton, &QPushButton::clicked, this, &MainWindow::onSettingClicked);

//    clickTimer.start();  //初始化计时器，记录首次点击时间

    leftLayout->addWidget(plotContainer, 3);
    leftLayout->addWidget(buttonContainer, 1);
    leftFrame->setLayout(leftLayout);

    // **右侧区域（带圆角边框）**
       QFrame *rightFrame = new QFrame();
       rightFrame->setStyleSheet(R"(
           QFrame {
               background-color: #1F1F1F;
               border-radius: 12px;
               border: 2px solid #444444;
           }
       )");
       QVBoxLayout *rightLayout = new QVBoxLayout(rightFrame);
       rightLayout->setContentsMargins(10, 10, 10, 10);
       rightLayout->setSpacing(15);

       // **颜色调整**
       QString panelStyle = "background-color: #222222; border-radius: 8px; border: 2px solid #444444;";
       QString valueStyle = "color: yellow; font-size: 48px; font-weight: bold; text-align: center; font-family: 黑体;";
       QString smallBoxStyle = "color: white; font-size: 20px; font-weight: bold; border: 2px solid #444444; background-color: #333333; font-family: 黑体;";

       auto createPressurePanel = [&](QString paramName, QString unit, QLabel*& label) -> PressurePanel {
           PressurePanel pressurePanel;

           pressurePanel.panel = new QFrame();
           pressurePanel.panel->setStyleSheet(panelStyle);
           pressurePanel.panel->setFixedHeight(100);

           QGridLayout *gridLayout = new QGridLayout(pressurePanel.panel);
           gridLayout->setContentsMargins(5, 5, 5, 5);
           gridLayout->setSpacing(2);

           pressurePanel.paramLabel = new QLabel(paramName);
           pressurePanel.paramLabel->setAlignment(Qt::AlignCenter);
           pressurePanel.paramLabel->setStyleSheet(smallBoxStyle);

           pressurePanel.unitLabel = new QLabel(unit);
           pressurePanel.unitLabel->setAlignment(Qt::AlignCenter);
           pressurePanel.unitLabel->setStyleSheet(smallBoxStyle);

           pressurePanel.valueLabel = new QLabel("---");  // **存储 valueLabel**
           pressurePanel.valueLabel->setAlignment(Qt::AlignCenter);
           pressurePanel.valueLabel->setStyleSheet(valueStyle);

           label = pressurePanel.valueLabel;  // **把 QLabel 指针赋值给外部变量**

           gridLayout->addWidget(pressurePanel.paramLabel, 0, 0, 1, 1);
           gridLayout->addWidget(pressurePanel.unitLabel, 1, 0, 1, 1);
           gridLayout->addWidget(pressurePanel.valueLabel, 0, 1, 2, 1);

           pressurePanel.panel->setLayout(gridLayout);

           return pressurePanel;
       };

       diastolicPanel = createPressurePanel(tr("舒张压"), "mmHg", diastolicValue);
       rightLayout->addWidget(diastolicPanel.panel);

       systolicPanel = createPressurePanel(tr("收缩压"), "mmHg", systolicValue);
       rightLayout->addWidget(systolicPanel.panel);

       avgPanel = createPressurePanel(tr("平均值"), "mmHg", avgValue);
       rightLayout->addWidget(avgPanel.panel);

       heartbeatPanel = createPressurePanel(tr("心率"), tr("次/分钟"), heartbeatValue);
       rightLayout->addWidget(heartbeatPanel.panel);



       rightFrame->setLayout(rightLayout);

       splitter->addWidget(leftFrame);
       splitter->addWidget(rightFrame);
       splitter->setSizes({leftWidth, rightWidth});

       mainLayout->addWidget(splitter);
       centralWidget->setLayout(mainLayout);
       setCentralWidget(centralWidget);
   }





MainWindow::~MainWindow()
{
    delete ui;
    dbManager.closeDatabase();
}

// 处理开始量测按钮点击事件
void MainWindow::onStartClicked() {
    qDebug() << "onStartClicked() called";

    // 判断量测标签是否为空
    if (!diastolicValue || !systolicValue || !avgValue || !heartbeatValue) {
        qDebug() << "Error: QLabel pointers are NULL!";
        return;  // 如果为空，则返回
    }

    if (isMeasuring) {
        // 如果量测正在进行，弹出提示窗口，询问是否暂停量测
        qDebug() << "Measurement is already in progress.";

        // 使用自定义提示窗口来询问是否暂停量测
        CustomMessageBox msgBox(this, tr("量测正在进行"), tr("量测正在进行，是否暂停量测？"), {tr("是"), tr("否")}, 200);
        msgBox.exec();  // 执行自定义消息框

        QString response = msgBox.getUserResponse();  // 获取用户响应

        if (response == tr("是")) {
            // 用户点击了 "是"，暂停量测
            qDebug() << "Pausing measurement...";

            // 切换量测按钮的文本和图标
            measureButton->setIcon(QIcon(":/image/start.png"));
            measureButton->setText(tr("开始量测"));

            // 停止量测
            timer->stop();
            isMeasuring = false;
            isPaused = true;  // 设置为暂停状态

            qDebug() << "Measurement paused.";
        } else {
            // 用户点击了 "否"，继续量测
            qDebug() << "Measurement continues.";
        }
    } else if (isPaused) {
        // 如果量测被暂停，恢复量测
        qDebug() << "Resuming measurement from time " << currentTime;

        // 开始定时器，继续从暂停的地方量测
        timer->start(1000);  // 每秒更新一次

        // 更新量测状态
        isMeasuring = true;
        isPaused = false;  // 恢复为非暂停状态

        // 切换量测按钮的文本和图标
        measureButton->setIcon(QIcon(":/image/stop.png"));
        measureButton->setText(tr("暂停量测"));

        qDebug() << "Measurement resumed.";
    } else {
        // 第一次点击开始量测
        qDebug() << "Starting measurement...";

        // 重置数据（清空旧的数据）
        resetMeasurementData();

        qDebug() << "Starting resetMeasurementData...";

        // 开始定时器，更新波形数据
        timer->start(1000);  // 每秒更新一次

        qDebug() << "Starting Timer...";

        // 更新量测状态
        isMeasuring = true;
        isPaused = false;  // 确保暂停状态被清除

        qDebug() << "updata measure status...";

        // 切换量测按钮的文本
        measureButton->setText(tr("暂停量测"));
        measureButton->setIcon(QIcon(":/image/stop.png"));


        qDebug() << "Starting Switch Text...";

        qDebug() << "Measurement started.";
    }
}


void MainWindow::resetMeasurementData() {
    currentTime = 0;
    xData.clear();
    yData.clear();

//    // 设置模拟量初始值
//    diastolicValue->setText("---");
//    systolicValue->setText("---");
//    avgValue->setText("---");
//    heartbeatValue->setText("---");
}

void MainWindow::onStopClicked() {
    if (isMeasuring) {
        // 弹出确认框，询问是否停止量测
        CustomMessageBox msgBox(this, tr("提示"), tr("量测正在进行，确定停止吗？"), {tr("是"), tr("否")}, 200);

        msgBox.exec();  // 显示提示框

        QString response = msgBox.getUserResponse();

        if (response == tr("是")) {
            qDebug() << "Measurement stopped by user.";

            // 停止计时器
            timer->stop();
            isMeasuring = false;
            isPaused = true;  // 设置为暂停状态

            // 清空显示
            diastolicValue->setText("---");
            systolicValue->setText("---");
            avgValue->setText("---");
            heartbeatValue->setText("---");

            // 更新按钮图标和文本
            measureButton->setIcon(QIcon(":/image/start.png"));
            measureButton->setText(tr("开始量测"));

            qDebug() << "Measurement stopped, displaying results.";
        } else {
            // 如果选择了“否”，继续量测
            qDebug() << "Measurement continues.";
        }
    } else {
        // 如果量测未进行，提示用户
        CustomMessageBox msgBox(this, tr("提示"), tr("量测尚未开始，无法停止。"), {tr("确定")}, 200);
        msgBox.exec();  // 执行消息框并显示
    }
}

void MainWindow::onTimerTimeout() {
    qDebug() << "Timer triggered: current time " << currentTime;
    currentTime++;  // 每次触发时，计时器时间增加

    if (currentTime <= 60) {  // 假设量测时间为60秒
        updateGraph();  // 更新图表数据

        // 实时计算当前数据的最小值、最大值、平均值和心率
                double currentMin = *std::min_element(yData.begin(), yData.end());
                double currentMax = *std::max_element(yData.begin(), yData.end());
                double currentAvg = currentMin + (1.0 / 3.0) * (currentMax - currentMin);
                int currentCycles = currentTime / 2;

                // 实时更新显示（使用QString::number指定小数位数）
                diastolicValue->setText(QString::number(currentMin, 'f', 1));
                systolicValue->setText(QString::number(currentMax, 'f', 1));
                avgValue->setText(QString::number(currentAvg, 'f', 1));
                heartbeatValue->setText(QString::number(currentCycles));
    }

    // 如果量测结束
    if (currentTime == 60) {
        onFinishMeasurement();
    }
}

// 完成测量后的处理
// 处理完成测量后的逻辑
void MainWindow::onFinishMeasurement()
{
    // 停止测量，弹出完成提示框
    timer->stop();
    isMeasuring = false;

    // 显示完成提示框
    CustomMessageBox msgBox(this, tr("测量已完成"), tr("测量已完成，点击确认查看结果"), {tr("确定")}, 200);
    msgBox.exec();  // 显示提示框

    qDebug() << "Measurement completed. Final values displayed.";

    // 切换按钮状态回默认状态“开始量测”
   measureButton->setIcon(QIcon(":/image/start.png"));  // 设置按钮图标为开始量测
    measureButton->setText(tr("开始量测"));  // 设置按钮文本为“开始量测”

}


// 更新图表数据
void MainWindow::updateGraph() {
    double frequency = 0.05; // 减小频率，确保波动足够明显
    double offset = 15.5;    // 偏移量

    // 随机化振幅，生成一个 5 到 10 之间的随机数
    std::random_device rd;  // 获取一个随机种子
    std::mt19937 gen(rd());  // 使用 Mersenne Twister 伪随机数生成器
    std::uniform_real_distribution<> dis(5.0, 7.0);  // 定义一个浮动的随机数范围

    double amplitude = dis(gen);  // 生成一个 5 到 10 之间的随机浮动数值

//    // 输出调试信息查看幅度是否变化
//    qDebug() << "Amplitude: " << amplitude;

    // 正弦波公式生成压力数据
    double pressure = amplitude * sin(2 * M_PI * frequency * currentTime) + offset;

    // 确保压力值在 6 到 25 之间
    pressure = qMin(qMax(pressure, 6.0), 25.0);

    // 更新最大值和最小值
    if (maxPressure == -1 || pressure > maxPressure) {
        maxPressure = pressure;
    }
    if (minPressure == -1 || pressure < minPressure) {
        minPressure = pressure;
    }

    // 输出当前的最大值和最小值
    qDebug() << "Current pressure: " << pressure << " Max pressure: " << maxPressure << " Min pressure: " << minPressure;

    // 更新数据
    yData.push_back(pressure);
    xData.push_back(currentTime);

    // 创建 QwtPointSeriesData 来包装 QVector<QPointF>
    QwtPointSeriesData *data = new QwtPointSeriesData(generateData());

    // 更新曲线数据
    curve->setData(data);  // 传递 QwtPointSeriesData 对象
    plot->replot();  // 更新图表
}


// 生成图表数据
QVector<QPointF> MainWindow::generateData() {
    QVector<QPointF> points;
    for (int i = 0; i < xData.size(); ++i) {
        points.append(QPointF(xData[i], yData[i]));
    }
    return points;
}

//上传数据到数据库
void MainWindow::onUploadDataClicked() {
    //量测过程中提示非法行为
    if (isMeasuring) {
        // 如果正在量测，弹出提示框告知用户无法上传数据
        CustomMessageBox msgBox(this, tr("错误"), tr("正在量测，无法上传数据！"), {tr("确定")}, 200);
        msgBox.exec();  // 显示自定义弹窗
        return;  // 阻止继续执行上传操作
    }

    // 获取显示的值
    QString diastolicText = diastolicValue->text();
    QString systolicText = systolicValue->text();
    QString avgText = avgValue->text();
    QString heartbeatText = heartbeatValue->text();

    qDebug() << "heartbeatText: " << heartbeatText;

    // 将输入框的值转换为数字
    bool diastolicOk, systolicOk, avgOk, heartbeatOk;
    double diastolic = diastolicText.toDouble(&diastolicOk);
    double systolic = systolicText.toDouble(&systolicOk);
    double avg = avgText.toDouble(&avgOk);
    int heartbeat = heartbeatText.toDouble(&heartbeatOk);


    qDebug() << "diastolicOk: " << diastolicOk
             << " systolicOk: " << systolicOk
             << " avgOk: " << avgOk
             << " heartbeatOk: " << heartbeatOk;


    // 判断数据是否合法
    if (!diastolicOk || !systolicOk || !avgOk || !heartbeatOk) {
        // 如果其中有任何一个参数不是数字，弹出提示框
        CustomMessageBox msgBox(this, tr("错误"), tr("数据不合法，请检查输入！"), {tr("确定")}, 200);
        msgBox.exec();  // 显示自定义弹窗
        return;
    }


    // 数据合法，插入到数据库
    if (dbManager.insertData(diastolic, systolic, avg, heartbeat)) {
        // 插入成功后，弹出提示框
        CustomMessageBox msgBox(this, tr("数据上传成功"), tr("数据上传成功！"), {tr("确定")}, 200);
        msgBox.exec();
    } else {
        // 插入失败后，弹出提示框
        CustomMessageBox msgBox(this, tr("数据上传失败"), tr("上传数据失败，请稍后再试！"), {tr("确定")}, 200);
        msgBox.exec();
    }
}

void MainWindow::onHistoryButtonClicked()
{
    // 判断是否正在量测
    if (isMeasuring) {
        // 如果正在量测，弹出提示框禁止查询
        CustomMessageBox msgBox(this, tr("操作禁止"), tr("量测正在进行，无法进行历史查询！"), {tr("确定")}, 225);
        msgBox.exec();
    } else {
        // 如果没有量测，可以进行历史查询
        HistoryCheck *historyWindow = new HistoryCheck(&dbManager, this);
        historyWindow->setAttribute(Qt::WA_DeleteOnClose);
        historyWindow->setGeometry(this->geometry());
        historyWindow->show();
    }
}


void MainWindow::onDataReceived(QByteArray data)
{
    qDebug() << "Received Data (Hex): " << data.toHex();  // 以十六进制格式打印接收到的数据
}


//void MainWindow::on_pushButtonSend_clicked()
//{
//    QByteArray command = QByteArray::fromHex("AA550102030455AA"); // 发送指令
//    serialManager->sendCommand(command);
//    qDebug() << "[Serial Sent] (Hex):" << command.toHex();
//}

void MainWindow::onErrorOccurred(QString errorMsg)
{
    qDebug() << "[Serial Error]: " << errorMsg;
}

//void MainWindow::onSettingClicked()
//{
//         static std::atomic<int> clickCount{0};
//        static QTimer *clickTimer = nullptr;

//        const int maxInterval = 2000;       // 每次点击间隔不能超过这个（最大点击窗口）
//        const int decisionDelay = 500;      // 等待下次点击前的决策延迟
//        static QElapsedTimer timer;

//        if (!clickTimer) {
//            clickTimer = new QTimer(this);
//            clickTimer->setSingleShot(true);
//            connect(clickTimer, &QTimer::timeout, this, [this]() {
//                if (clickCount < 5) {
//                    qDebug() << "打开设置界面";

//                    SettingsWidget *settingswidget = new SettingsWidget(this);
//                    settingswidget->setAttribute(Qt::WA_DeleteOnClose); // 自动销毁
//                    settingswidget->show();//打开正常的设置界面
//                }
//                clickCount = 0;  // 重置点击次数
//            });
//        }

//        // 判断时间间隔
//        if (timer.isValid() && timer.elapsed() > maxInterval) {
//            clickCount = 0;  // 超过最大间隔，重置计数
//        }

//        clickCount++;
//        timer.restart();

//        if (clickCount >= 5) {
//            qDebug() << "触发维护界面";
//            clickTimer->stop();     // 取消打开设置界面的等待
//            showHiddenWidget();     // 显示隐藏界面
//            clickCount = 0;
//        } else {
//            clickTimer->start(decisionDelay);  // 等待是否还有下一次点击
//        }

//}

void MainWindow::showHiddenWidget() {
    if (!hiddenWidget) {  //如果隐藏界面不存在，则创建
        hiddenWidget = new MaintenanceWidget();
        hiddenWidget->setFixedSize(1024, 600);
    }
    hiddenWidget->show();
}


//void MainWindow::changeLanguage(const QString &languageCode)
//{
//    qApp->removeTranslator(&translator);

//    QString qmPath = ":/translations/translations/" + languageCode + ".qm";

//    if (translator.load(qmPath)) {
//        qApp->installTranslator(&translator);
//        qDebug() << "语言切换成功：" << qmPath;

//        // **存储用户选择的语言**
//        QSettings settings("MyCompany", "MyApp");
//        settings.setValue("language", languageCode);
//    } else {
//        qDebug() << "语言加载失败：" << qmPath;
//    }

//    // **手动更新 UI**
//    ui->retranslateUi(this);

//    // **遍历所有顶级窗口，发送 `LanguageChange` 事件**
//    QEvent event(QEvent::LanguageChange);
//    foreach (QWidget *widget, QApplication::topLevelWidgets()) {
//        QApplication::sendEvent(widget, &event);
//    }

//    qDebug() << "语言切换事件已发送，所有界面应该自动更新！";
//}


void MainWindow::changeEvent(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) {
        qDebug() << "Language Change Event Triggered!";

//        ui->retranslateUi(this);
//        this->setWindowTitle(tr("主操作界面"));

        //延迟执行 UI 语言更新，避免 UI 未初始化问题
        QTimer::singleShot(0, this, &MainWindow::applyLanguageChange);
    }

    QWidget::changeEvent(event);
}

void MainWindow::applyLanguageChange() {
    qDebug() << "Applying language change...";

    QSettings settings("MyCompany", "MyApp");
    QString newLanguageCode = settings.value("language", "zh_CN").toString();  //读取语言
    qDebug() << "Loaded language from settings:" << newLanguageCode;

    // 避免重复切换
    static QString currentLanguage = "";
    if (currentLanguage == newLanguageCode) {
        qDebug() << "Language is already set to:" << newLanguageCode << ", skipping change.";
        return;
    }

    currentLanguage = newLanguageCode;  //更新当前语言

    static QTranslator translator;
    qApp->removeTranslator(&translator);

    QString translationPath = ":/translations/translations/" + newLanguageCode + ".qm";
    if (translator.load(translationPath)) {
        qApp->installTranslator(&translator);
        qDebug() << "Language switched to:" << newLanguageCode;
    } else {
        qDebug() << "Failed to load language file:" << translationPath;
    }

    //记录语言到 QSettings（但不触发额外事件）
    settings.setValue("language", newLanguageCode);


    // 确保指针有效后再更新 UI
    if (measureButton) {
        measureButton->setText(tr("开始测量"));  // **未操作时显示默认文本**
    } else {
        measureButton->setText(isMeasuring ? tr("暂停测量") : tr("开始测量"));
    }
    if (uploadButton) uploadButton->setText(tr("上传数据"));
    if (historyButton) historyButton->setText(tr("历史查询"));
    if (settingsButton) settingsButton->setText(tr("参数调节"));

    // 更新 PressurePanel 里的文字
    if (diastolicPanel.paramLabel)diastolicPanel.paramLabel->setText(tr("舒张压"));
    if (systolicPanel.paramLabel)systolicPanel.paramLabel->setText(tr("收缩压"));
    if (avgPanel.paramLabel)avgPanel.paramLabel->setText(tr("平均值"));
    if(heartbeatPanel.paramLabel)heartbeatPanel.paramLabel->setText(tr("心率"));
    if(heartbeatPanel.unitLabel)heartbeatPanel.unitLabel->setText(tr("次/分钟"));

    //更新plot的坐标
    if (plot) {
        // **获取当前字体**
        QFont currentFont = plot->axisTitle(QwtPlot::xBottom).font();

        // **更新 X 轴标题，并应用原来的字体**
        QwtText xTitle(tr("时间 (s)"));
        xTitle.setFont(currentFont);
        plot->setAxisTitle(QwtPlot::xBottom, xTitle);

        // **更新 Y 轴标题，并应用原来的字体**
        QwtText yTitle(tr("压力 (mmHg)"));
        yTitle.setFont(currentFont);
        plot->setAxisTitle(QwtPlot::yLeft, yTitle);

        // 重新绘制
        plot->replot();
    }


}






