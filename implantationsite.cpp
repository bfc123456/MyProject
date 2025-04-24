#include "implantationsite.h"
#include "calibrationdialog.h"
#include "implantmonitor.h"
#include <QDebug>
#include <QMessageBox>
#include <QGraphicsBlurEffect>
#include <QTimer>
#include <QSqlQuery>
#include <QSqlError>

ImplantationSite::ImplantationSite( QWidget* parent , const QString &sensorId)
    : QWidget(parent), m_serial(sensorId){

    this->setStyleSheet(R"(
        QWidget {
             background-color: qlineargradient(
                 x1: 0, y1: 0, x2: 0, y2: 1,
                 stop: 0 rgba(30, 50, 80, 0.9),     /* 顶部：偏亮蓝灰，透明度 0.9 */
                 stop: 1 rgba(10, 25, 50, 0.75)     /* 底部：深蓝，透明度 0.75 */
             );
            color: white;
            font-size: 14px;
            border-radius: 10px;
        }

        QFrame#TopBar{
            background-color: qlineargradient(
                x1: 0, y1: 0, x2: 0, y2: 1,
                stop: 0 rgba(25, 50, 75, 0.9),
                stop: 1 rgba(10, 20, 30, 0.85)
            );
            border-radius: 10px;
            border: 1px solid rgba(255, 255, 255, 0.08); /* 边缘高光 */
        }

        QFrame#frameHeart{
            background-color: qlineargradient(
                x1: 0, y1: 0, x2: 0, y2: 1,
                stop: 0 rgba(25, 50, 75, 0.9),
                stop: 1 rgba(10, 20, 30, 0.85)
            );
            border-radius: 10px;
            border: 1px solid rgba(255, 255, 255, 0.08); /* 边缘高光 */
        }

        QFrame#framePressure{
            background-color: qlineargradient(
                x1: 0, y1: 0, x2: 0, y2: 1,
                stop: 0 rgba(25, 50, 75, 0.9),
                stop: 1 rgba(10, 20, 30, 0.85)
            );
            border-radius: 10px;
            border: 1px solid rgba(255, 255, 255, 0.08); /* 边缘高光 */
        }


        QFrame#frameSignal{
            background-color: qlineargradient(
                x1: 0, y1: 0, x2: 0, y2: 1,
                stop: 0 rgba(25, 50, 75, 0.9),
                stop: 1 rgba(10, 20, 30, 0.85)
            );
            border-radius: 10px;
            border: 1px solid rgba(255, 255, 255, 0.08); /* 边缘高光 */
        }
    )");


    mainLayout = new QVBoxLayout(this);

    /********** 第一行：标题栏 **********/
    QFrame* frameTop = new QFrame(this);
    frameTop->setObjectName("TopBar");
    frameTop->setFrameShape(QFrame::StyledPanel);
    frameTop->setFixedHeight(45);

    QHBoxLayout* topLayout = new QHBoxLayout(frameTop);
    QLabel* labelTitle = new QLabel(tr("新植入物"), frameTop);
    labelTitle->setStyleSheet("font-weight: bold; font-size: 16px; background-color: transparent; color: white;");
    QLabel* labelSN = new QLabel("XXXXXXXX", frameTop);
    labelSN->setStyleSheet("font-weight: bold; font-size: 16px; background-color: transparent; color: white;");
    QPushButton* settingsButton = new QPushButton(frameTop);

    labelTitle->setAlignment(Qt::AlignCenter);
    labelSN->setAlignment(Qt::AlignCenter);

    settingsButton->setFixedSize(25, 25);
    settingsButton->setIcon(QIcon(":/image/icons8-shezhi.png"));
    settingsButton->setIconSize(QSize(24, 24));
    settingsButton->setStyleSheet("QPushButton { border: none; background-color: transparent; border-radius: 20px; } QPushButton:pressed { background-color: rgba(255, 255, 255, 0.2); }");
    connect(settingsButton, &QPushButton::clicked, this, &ImplantationSite::OpenSettingsRequested);

    topLayout->addWidget(labelTitle);
    topLayout->addStretch();
    topLayout->addWidget(labelSN);
    topLayout->addStretch();
    topLayout->addWidget(settingsButton);

    mainLayout->addWidget(frameTop);

    /********** 第二行：左侧植入部位，右侧曲线图 **********/
    QHBoxLayout* secondRow = new QHBoxLayout();

    // 左侧植入图
    QFrame* frameImplant = new QFrame(this);
    frameImplant->setObjectName("frameImplant");
    frameImplant->setFixedSize(460, 320);
    frameImplant->setStyleSheet("QFrame { background-image: url(:/image/body.png); background-position: center; background-repeat: no-repeat; border: none; }");
    QVBoxLayout* implantLayout = new QVBoxLayout(frameImplant);

    QLabel* implantTitle = new QLabel(tr("输入传感器植入位置"), frameImplant);
    implantTitle->setFixedSize(300, 50);
    implantTitle->setAlignment(Qt::AlignCenter);
    implantTitle->setStyleSheet("QLabel { background-image: url(:/image/OIP.jpg); color: white; font-size: 20px; border: none; }");

    buttonL = new QPushButton(tr("左"), frameImplant);
    buttonR = new QPushButton(tr("右"), frameImplant);

    connect(buttonL,&QPushButton::clicked,this,&ImplantationSite::onBtnLocationClicked);
    connect(buttonR,&QPushButton::clicked,this,&ImplantationSite::onBtnLocationClicked);

    buttonL->setFixedSize(80, 80);
    buttonR->setFixedSize(80, 80);
    QString buttonStyle = "QPushButton { background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #8899aa, stop:1 #556677); color: white; font-weight: bold; font-size: 18px; border-radius: 10px; } QPushButton:pressed { background-color: #445566; }";
    buttonL->setStyleSheet(buttonStyle);
    buttonR->setStyleSheet(buttonStyle);

    QHBoxLayout* lrButtonLayout = new QHBoxLayout();
    lrButtonLayout->addWidget(buttonL);
    lrButtonLayout->addWidget(buttonR);
    lrButtonLayout->setSpacing(55);

    implantLayout->addWidget(implantTitle, 0, Qt::AlignCenter);
    implantLayout->addLayout(lrButtonLayout);
    implantLayout->addSpacing(60);
    secondRow->addWidget(frameImplant);

    // 右侧曲线图
    QFrame* frameCurve = new QFrame(this);
    frameCurve->setObjectName("frameCurve");
    frameCurve->setFixedSize(460, 320);
    QVBoxLayout* curveLayout = new QVBoxLayout(frameCurve);

    QHBoxLayout* curveTitleLayout = new QHBoxLayout();
    QLabel* labelCurve = new QLabel(tr("实时波形监测"), frameCurve);
    labelCurve->setStyleSheet("font-weight: bold; font-size: 16px; background-color: transparent; color: white;");
    QPushButton* helpButton = new QPushButton(tr("帮助"), frameCurve);
    helpButton->setIcon(QIcon(":/image/icons8-help.png"));
    helpButton->setIconSize(QSize(13, 13));
    helpButton->setFixedSize(75, 23);
    helpButton->setStyleSheet("QPushButton { background-color: rgba(255,0,0,0.6); color: white; font-weight: bold; } QPushButton:pressed { background-color: red; }");
    connect(helpButton, &QPushButton::clicked, this, [this]() {
        // 创建QDialog作为自定义消息框
        QDialog *dialog = new QDialog(this);
        dialog->setFixedSize(400,200);
        dialog->setWindowTitle("操作提示");

        // 创建消息标签
        QLabel *messageLabel1 = new QLabel(tr("1. 移动传感器至天线中心，然后缓慢移动"), dialog);
        QLabel *messageLabel2 = new QLabel(tr("2. 确认信号强度逐渐变强然后停止，并重复三次"), dialog);
//        messageLabel->setWordWrap(true);  // 允许自动换行
        messageLabel1->setStyleSheet("background-color: transparent; color: white; font-size: 16px;");  // 设置字体和颜色
        messageLabel2->setStyleSheet("background-color: transparent; color: white; font-size: 16px;");  // 设置字体和颜色

        // 创建确认按钮
        QPushButton *confirmButton = new QPushButton(tr("确认"), dialog);

        // 设置按钮的样式
        confirmButton->setStyleSheet(R"(
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
        confirmButton->setFixedWidth(100);

        // 创建垂直布局
        QVBoxLayout *layout = new QVBoxLayout(dialog);
        layout->addSpacing(20);
        layout->addWidget(messageLabel1,0,Qt::AlignLeft);  // 添加消息
        layout->addSpacing(20);
        layout->addWidget(messageLabel2,0,Qt::AlignLeft);  // 添加消息
        layout->addSpacing(20);
        layout->addWidget(confirmButton,0,Qt::AlignCenter);  // 添加确认按钮

        // 连接按钮点击事件
        connect(confirmButton, &QPushButton::clicked, dialog, &QDialog::accept);  // 点击按钮后接受对话框

        dialog->exec();  // 显示对话框
    });


    curveTitleLayout->addWidget(labelCurve);
    curveTitleLayout->addStretch();
    curveTitleLayout->addWidget(helpButton);

    plot = new QwtPlot(frameCurve);
    plot->setCanvasBackground(QColor(31, 31, 31));
    plot->setAxisScale(QwtPlot::xBottom, 0, 10);
    plot->setAxisScale(QwtPlot::yLeft, 0, 3);

    // 添加网格
    grid = new QwtPlotGrid();
    grid->enableX(false);
    grid->setPen(QPen(Qt::lightGray, 0, Qt::DotLine));  // 设置网格的颜色为浅灰色，点状线
    grid->attach(plot); // 将网格附加到绘图区域

        // 设置曲线
        curve = new QwtPlotCurve();
        curve->setPen(QPen(Qt::cyan, 2));  // 设置线条颜色和宽度
        curve->setStyle(QwtPlotCurve::Lines);  // 曲线样式
        curve->setBrush(QBrush(QColor(0, 255, 255, 50))); // 设置填充颜色，50%的透明度
        curve->attach(plot);

        // 初始化定时器
        timer = new QTimer(this);
        timer->setInterval(1000);
        connect(timer, &QTimer::timeout, this, &ImplantationSite::updatePlot);
        timer->start(100);  // 每 100 毫秒更新一次图表

     startSimulation();   // 启动模拟


    curveLayout->addLayout(curveTitleLayout);
    curveLayout->addWidget(plot);
    secondRow->addWidget(frameCurve);
    mainLayout->addLayout(secondRow);

    /********** 第三行：心率、血压、信号 **********/
    QHBoxLayout* thirdRow = new QHBoxLayout();

    // 心率
    QFrame* frameHeart = new QFrame(this);
    frameHeart->setObjectName("frameHeart");
    frameHeart->setFixedHeight(120);
    QVBoxLayout* heartLayout = new QVBoxLayout(frameHeart);
    QLabel* heartTitle = new QLabel(tr("心率监测"), frameHeart);
    heartTitle->setStyleSheet("font-weight: bold; font-size: 16px; background-color: transparent; color: white;");
    heartTitle->setAlignment(Qt::AlignCenter);
    QLabel* heartIcon = new QLabel(frameHeart);
    heartIcon->setPixmap(QPixmap(":/image/icons8-heart.png").scaled(16, 16));
    heartIcon->setStyleSheet("font-weight: bold; font-size: 16px; background-color: transparent; color: white;");
    QLabel* heartValue = new QLabel("78", frameHeart);
    QLabel* heartUnit = new QLabel(tr("次/分钟"), frameHeart);
    heartValue->setStyleSheet("font-size:22px; color:#3399ff; font-weight:bold;");
    heartUnit->setStyleSheet("color:#888;");

    QHBoxLayout* heartTitleLayout = new QHBoxLayout();
    heartTitleLayout->addWidget(heartTitle);
    heartTitleLayout->addStretch();
    heartTitleLayout->addWidget(heartIcon);

    heartLayout->addLayout(heartTitleLayout);
    heartLayout->addWidget(heartValue);
    heartLayout->addWidget(heartUnit);
    thirdRow->addWidget(frameHeart);

    // 血压
    QFrame* framePressure = new QFrame(this);
    framePressure->setObjectName("framePressure");
    framePressure->setFixedHeight(120);
    QVBoxLayout* pressureLayout = new QVBoxLayout(framePressure);
    QLabel* pressureTitle = new QLabel(tr("血压监测"), framePressure);
    pressureTitle->setStyleSheet("font-weight: bold; font-size: 16px; background-color: transparent; color: white;");
    QLabel* pressureIcon = new QLabel(framePressure);
    pressureIcon->setPixmap(QPixmap(":/image/icons8-tingzhen.png").scaled(16, 16));
    pressureIcon->setStyleSheet("font-weight: bold; font-size: 16px; background-color: transparent; color: white;");
    QLabel* pressureValue = new QLabel("120/80", framePressure);
    QLabel* pulseValue = new QLabel("94", framePressure);
    QLabel* pressureUnit = new QLabel("mmHg", framePressure);
    pressureValue->setStyleSheet("font-size:20px; color:#3399ff; font-weight:bold;");
    pulseValue->setStyleSheet("font-size:20px; color:#3399ff; font-weight:bold;");
    pressureUnit->setStyleSheet("color:#999;");

    QHBoxLayout* pressureTopLayout = new QHBoxLayout();
    pressureTopLayout->addWidget(pressureTitle);
    pressureTopLayout->addStretch();
    pressureTopLayout->addWidget(pressureIcon);

    QHBoxLayout* pressureValueLayout = new QHBoxLayout();
    pressureValueLayout->addWidget(pressureValue);
    pressureValueLayout->addSpacing(10);
    pressureValueLayout->addWidget(pulseValue);

    pressureLayout->addLayout(pressureTopLayout);
    pressureLayout->addLayout(pressureValueLayout);
    pressureLayout->addWidget(pressureUnit);
    thirdRow->addWidget(framePressure);

    // 信号强度
    QFrame* frameSignal = new QFrame(this);
    frameSignal->setObjectName("frameSignal");
    frameSignal->setFixedHeight(120);
    QVBoxLayout* signalLayout = new QVBoxLayout(frameSignal);
    QLabel* signalTitle = new QLabel(tr("信号强度"), frameSignal);
    signalTitle->setStyleSheet("font-weight: bold; font-size: 16px; background-color: transparent; color: white;");
    QLabel* signalIcon = new QLabel(frameSignal);
    signalIcon->setPixmap(QPixmap(":/image/icons8-signal.png").scaled(16, 16));
    signalIcon->setStyleSheet("font-weight: bold; font-size: 16px; background-color: transparent; color: white;");
    progress = new CircularProgressBar(frameSignal);
    progress->setProgress(90);
    progress->setFixedSize(65, 65);

    QHBoxLayout* signalTopLayout = new QHBoxLayout();
    signalTopLayout->addWidget(signalTitle);
    signalTopLayout->addStretch();
    signalTopLayout->addWidget(signalIcon);

    signalLayout->addLayout(signalTopLayout);
    signalLayout->addWidget(progress, 0, Qt::AlignHCenter);
    thirdRow->addWidget(frameSignal);

    mainLayout->addLayout(thirdRow);

    /********** 第四行：按钮操作 **********/
    QHBoxLayout* fourthRow = new QHBoxLayout();

    QPushButton* returnButton = new QPushButton(tr("返回上级"), this);
    returnButton->setFixedSize(120, 45);
    returnButton->setIcon(QIcon(":/image/icons8-return.png"));
    connect(returnButton, &QPushButton::clicked, this, [this]() {
        emit returnRequested();
    });

    returnButton->setStyleSheet(R"(
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


    QPushButton* calibrateButton = new QPushButton(tr("校准数据"), this);
    calibrateButton->setFixedSize(120, 45);
    calibrateButton->setIcon(QIcon(":/image/icons8-calibration.png"));
    connect(calibrateButton, &QPushButton::clicked, this, [this]() {
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

        CalibrationDialog *calibrationialog = new CalibrationDialog(this);
        qDebug()<<"121";
        connect(calibrationialog,&CalibrationDialog::openmonitorwidget,this,[this](){
            ImplantMonitor *implantmonitor = new ImplantMonitor();
            implantmonitor->show();
            this->hide();
        });
        calibrationialog->setAttribute(Qt::WA_DeleteOnClose);       // 自动销毁

        // 监听关闭信号
        connect(calibrationialog, &QDialog::finished, this, [=]() {
            this->setGraphicsEffect(nullptr);
            overlay->close();
            overlay->deleteLater();
        });

        calibrationialog->show();
    });

    calibrateButton->setStyleSheet(R"(
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

    fourthRow->addWidget(returnButton);
    fourthRow->addStretch();
    fourthRow->addWidget(calibrateButton);
    mainLayout->addLayout(fourthRow);
}

ImplantationSite::~ImplantationSite() {
    
    delete timer;
    delete curve;
    delete plot;
    
}

void ImplantationSite::OpenSettingsRequested() {
    settingswidget = new SettingsWidget();
    settingswidget->setWindowFlags(Qt::Dialog);
    settingswidget->setAttribute(Qt::WA_DeleteOnClose);
    settingswidget->show();
}

//void ImplantationSite::openImplantMonitorWidget() {
//    ImplantMonitor* implantmonitor = new ImplantMonitor(this);
//    implantmonitor->setWindowFlags(Qt::Window);
//    implantmonitor->setFixedSize(1024, 600);
//    implantmonitor->show();
//}

void ImplantationSite::startSimulation() {
    // 每次定时更新数据和绘图
    data.resize(100);  // 确保数据大小一致
    for (int i = 0; i < data.size(); ++i) {
        data[i] = qrand() % 100 / 100.0;  // 初始化随机数据
    }
}

void ImplantationSite::updateData() {
    // 模拟数据变化，更新传感器信号
    double lastValue = data.last();
    double newValue = lastValue + (qrand() % 20 - 10) / 100.0;  // 随机信号变化
    if (newValue < 0) newValue = 0;   // 保证数据不小于 0
    if (newValue > 5) newValue = 5;   // 保证数据不大于 5
    data.append(newValue);  // 添加新的数据点

    // 保证数据量一致，删除多余的点
    if (data.size() > 100) {
        data.removeFirst();
    }

    // 更新信号强度百分比（0到100之间）
    int signalPercentage = static_cast<int>((newValue / 3.0) * 100); // 将信号强度转换为百分比

    progress->setProgress(signalPercentage);  // 更新进度条的显示
}


void ImplantationSite::updatePlot() {
    updateData();  // 更新数据

    QVector<QPointF> points;
    for (int i = 0; i < data.size(); ++i) {
        points.append(QPointF(i, data[i]));  // 将 data 转换为 QPointF 格式
    }

    QwtPointSeriesData* seriesData = new QwtPointSeriesData(points);
    curve->setData(seriesData);  // 设置数据

    plot->replot();  // 重绘图表
}

void ImplantationSite::onBtnLocationClicked()
{
    //判断是哪个按钮
    auto *btn = qobject_cast<QPushButton*>(sender());
    QString loc;
    if (btn == buttonL)  loc = "left";
    else if (btn == buttonR) loc = "right";
    else return;

    //确认弹窗
    CustomMessageBox dlg(
        this,
        tr("确认上传"),
        tr("是否上传已选择的位置？"),
        { tr("确定"), tr("返回") },
        350
    );
    dlg.exec();
    if (dlg.getUserResponse() != tr("确定"))
        return;

    //执行上传
    if (uploadLocation(loc)) {
        CustomMessageBox ok(this, tr("成功"),
            tr("位置已上传：%1").arg(loc=="left"?tr("左侧"):tr("右侧")),
            { tr("确定") }, 300);
        ok.exec();
        ok.close();
    }
}

bool ImplantationSite::uploadLocation(const QString &loc)
{
    QSqlQuery q;
    q.prepare(R"(
      UPDATE sensor_info
         SET location = :loc
       WHERE sensor_id = :id
    )");
    q.bindValue(":loc", loc);
    q.bindValue(":id",  m_serial);
    return q.exec();
}


