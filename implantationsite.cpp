#include "implantationsite.h"
#include <QDebug>
#include <QMessageBox>
#include <QGraphicsBlurEffect>
#include <QTimer>
#include <QSqlQuery>
#include <QSqlError>
#include <QGuiApplication>
#include <QScreen>

ImplantationSite::ImplantationSite( QWidget* parent , const QString &sensorId)
    : FramelessWindow(parent), m_serial(sensorId){

    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int screenWidth = screenGeometry.width();
    int screenHeight = screenGeometry.height();

    // 计算缩放比例
    scaleX = (float)screenWidth / 1024;
    scaleY = (float)screenHeight / 600;

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
    mainLayout->setContentsMargins(30*scaleX,5*scaleY,30*scaleX,5*scaleY);

    /********** 第一行：标题栏 **********/
    QWidget * topwidget  = new QWidget(this);
    topwidget->setObjectName("TopBar");
    topwidget->setStyleSheet(R"(
        QWidget#TopBar {
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
    topwidget->setFixedHeight(50*scaleY);

    titleLabel = new QLabel(tr("新植入物"));
    titleLabel->setStyleSheet("background-color: transparent; color: white;");
    idLabel = new QLabel();
    idLabel->setText(m_serial);
    idLabel->setStyleSheet("background-color: transparent; color: white;");
    titleLabel->setFixedWidth(120*scaleY);
    titleLabel->setAlignment(Qt::AlignCenter);
    idLabel->setFixedWidth(120*scaleY);
    idLabel->setAlignment(Qt::AlignCenter);

    QPushButton *btnSettings = new QPushButton(this);
    btnSettings->setIcon(QIcon(":/image/icons8-shezhi.png"));
    btnSettings->setIconSize(QSize(24*scaleX, 24*scaleY));
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

    connect(btnSettings, &QPushButton::clicked, this, &ImplantationSite::OpenSettingsRequested);

    QHBoxLayout *topLayout = new QHBoxLayout(topwidget);
    topLayout->addWidget(titleLabel, 0, Qt::AlignLeft);

    // idLabel 拉伸份额为 1，并且居中
    topLayout->addWidget(idLabel, 1, Qt::AlignCenter);

    // btnSettings 不拉伸，强制右对齐
    topLayout->addWidget(btnSettings, 0, Qt::AlignRight);


    mainLayout->addWidget(topwidget);

    /********** 第二行：左侧植入部位，右侧曲线图 **********/
    QHBoxLayout* secondRow = new QHBoxLayout();
    secondRow->setContentsMargins(30*scaleX,0,30*scaleX,0);

    // 左侧植入图
    QFrame* frameImplant = new QFrame(this);
    frameImplant->setObjectName("frameImplant");
    frameImplant->setFixedSize(430*scaleX, 320*scaleY);

    frameImplant->setStyleSheet(R"(
        QWidget#frameImplant {
            border: none;
            /* 用 border-image 拉伸背景 */
            border-image: url(:/image/newbody.png) 0 0 0 0 stretch stretch;
        }
    )");
    frameImplant->setObjectName("frameImplant");

    QVBoxLayout* implantLayout = new QVBoxLayout(frameImplant);

    implantTitle = new QLabel(tr("请点击按钮选择植入位置"), frameImplant);
    implantTitle->setFixedSize(350*scaleX, 50*scaleY);
    implantTitle->setAlignment(Qt::AlignCenter);

    implantTitle->setStyleSheet(R"(
        QLabel {
            background-color: transparent;
            color: white;
            font-size: 20px;
            border: none;
        }
    )");

    buttonL = new QPushButton(tr("左"), frameImplant);
    buttonR = new QPushButton(tr("右"), frameImplant);


    connect(buttonL,&QPushButton::clicked,this,[this](){
        onBtnLocationClicked();
    });
    connect(buttonR,&QPushButton::clicked,this,[this](){
        onBtnLocationClicked();
    });

    buttonL->setFixedSize(80*scaleX, 80*scaleY);
    buttonR->setFixedSize(80*scaleX, 80*scaleY);

    QString buttonStyle = R"(
    QPushButton {
        /* 主色调：深一点、更柔和 */
        background-color: rgba(33, 150, 243, 0.85);  /* #2196F3 + 85% 不透明度 */
        color: #F0F0F0;
        font-weight: 600;
        font-size: 16px;
        border: none;
        border-radius: 8px;
        padding: 5px 12px;
    }
    QPushButton:pressed {
        /* 再深一点 */
        background-color: rgba(21, 101, 192, 1.0);  /* #1565C0 */
    }
    )";
    buttonL->setStyleSheet(buttonStyle);
    buttonR->setStyleSheet(buttonStyle);

    QHBoxLayout* lrButtonLayout = new QHBoxLayout();
    lrButtonLayout->addWidget(buttonL);
    lrButtonLayout->addWidget(buttonR);
    lrButtonLayout->setSpacing(20*scaleX);

    implantLayout->addSpacing(60*scaleY);
    implantLayout->addWidget(implantTitle, 0, Qt::AlignCenter);
    implantLayout->addLayout(lrButtonLayout);
    implantLayout->addSpacing(60*scaleY);
    secondRow->addWidget(frameImplant);

    // 右侧曲线图
    QFrame* frameCurve = new QFrame(this);
    frameCurve->setObjectName("frameCurve");
    frameCurve->setFixedSize(430*scaleX, 320*scaleY);
    QVBoxLayout* curveLayout = new QVBoxLayout(frameCurve);

    QHBoxLayout* curveTitleLayout = new QHBoxLayout();
    labelCurve = new QLabel(tr("实时波形监测"), frameCurve);
    labelCurve->setStyleSheet("font-weight: bold; font-size: 16px; background-color: transparent; color: white;");
    helpButton = new QPushButton(tr("帮助"), frameCurve);
    helpButton->setIcon(QIcon(":/image/icons8-help.png"));
    helpButton->setIconSize(QSize(13*scaleX, 13*scaleY));
    helpButton->setFixedSize(75*scaleX, 23*scaleY);
    helpButton->setStyleSheet("QPushButton { background-color: rgba(255,0,0,0.6); color: white; font-weight: bold; } QPushButton:pressed { background-color: red; }");

    connect(helpButton, &QPushButton::clicked, this, [this]() {
        // 创建QDialog作为自定义消息框
        CustomMessageBox dlg(
            this,
            tr("操作提示"),
            tr("1. 移动传感器至天线中心，然后缓慢移动\n\n"
               "2. 确认信号强度逐渐变强然后停止，并重复三次"),
            { tr("确认") },
            400*scaleY  // 对话框宽度
        );

        dlg.exec();
    });


    curveTitleLayout->addWidget(labelCurve);
    curveTitleLayout->addStretch();
    curveTitleLayout->addWidget(helpButton);

    plot = new ModernWavePlot(frameCurve);
    plot->setCanvasBackground(QColor(31, 31, 31));
    plot->setAxisScale(QwtPlot::xBottom, 0, 10);
    plot->setAxisScale(QwtPlot::yLeft, 0, 3);
    plot->setAutoFillBackground(false);
    plot->setStyleSheet("background: transparent; border: none; color:white;");

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
    secondRow->setSpacing(40*scaleX);
    secondRow->addWidget(frameCurve);
    mainLayout->addLayout(secondRow);

    /********** 第三行：心率、血压、信号 **********/
    QHBoxLayout* thirdRow = new QHBoxLayout();

    // 心率
    QFrame* frameHeart = new QFrame(this);
    frameHeart->setObjectName("frameHeart");
    frameHeart->setFixedHeight(120*scaleY);
    QVBoxLayout* heartLayout = new QVBoxLayout(frameHeart);
    heartTitle = new QLabel(tr("心率监测"), frameHeart);
    heartTitle->setStyleSheet("font-weight: bold; font-size: 16px; background-color: transparent; color: white;");
    heartTitle->setAlignment(Qt::AlignCenter);
    QLabel* heartIcon = new QLabel(frameHeart);
    heartIcon->setPixmap(QPixmap(":/image/icons8-heart.png").scaled(16, 16));
    heartIcon->setStyleSheet("font-weight: bold; font-size: 16px; background-color: transparent; color: white;");
    QLabel* heartValue = new QLabel("78", frameHeart);
    heartUnit = new QLabel(tr("次/分钟"), frameHeart);
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
    framePressure->setFixedHeight(120*scaleY);
    QVBoxLayout* pressureLayout = new QVBoxLayout(framePressure);
    pressureTitle = new QLabel(tr("血压监测"), framePressure);
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
    pressureValueLayout->addSpacing(10*scaleX);
    pressureValueLayout->addWidget(pulseValue);

    pressureLayout->addLayout(pressureTopLayout);
    pressureLayout->addLayout(pressureValueLayout);
    pressureLayout->addWidget(pressureUnit);
    thirdRow->addWidget(framePressure);

    // 信号强度
    QFrame* frameSignal = new QFrame(this);
    frameSignal->setObjectName("frameSignal");
    frameSignal->setFixedHeight(120*scaleY);
    QVBoxLayout* signalLayout = new QVBoxLayout(frameSignal);
    signalTitle = new QLabel(tr("信号强度"), frameSignal);
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

    returnButton = new QPushButton(tr("返回上级"), this);
    returnButton->setFixedSize(120*scaleX, 45*scaleY);
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
        padding: 4px 10px;
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


    calibrateButton = new QPushButton(tr("校准数据"), this);
    calibrateButton->setFixedSize(120*scaleX, 45*scaleY);
    calibrateButton->setIcon(QIcon(":/image/icons8-calibration.png"));
    connect(calibrateButton, &QPushButton::clicked, this, [this]() {
    //添加遮罩层
    QWidget *overlay = new QWidget(this);
    overlay->setStyleSheet("background-color: rgba(0, 0, 0, 100);"); // 可调透明度
    overlay->setAttribute(Qt::WA_TransparentForMouseEvents, true);  //不拦截
    overlay->show();
    overlay->raise();

    //添加模糊效果
    QGraphicsBlurEffect *blur = new QGraphicsBlurEffect;
    blur->setBlurRadius(20);  // 可调强度：20~40
    this->setGraphicsEffect(blur);

    if(!calibrationialog)calibrationialog = new CalibrationDialog(this);
    connect(calibrationialog,&CalibrationDialog::openmonitorwidget,this,[this](){
        if(!implantmonitor){
            implantmonitor = new ImplantMonitor(nullptr,m_serial);
            connect(implantmonitor,&ImplantMonitor::returnImplantationsite,this,[this](){
//            qDebug()<<"槽函数已触发，植入位置界面打开";
            this->show();
            });
            }
        implantmonitor->show();
        this->hide();
    });

    // 监听关闭信号
    connect(calibrationialog, &QDialog::finished, this, [=]() {
        this->setGraphicsEffect(nullptr);
        overlay->close();
        overlay->deleteLater();
        calibrationialog->deleteLater();
        calibrationialog = nullptr;    // 避免悬空
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
        padding: 4px 10px;
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

void ImplantationSite::onBtnLocationClicked() {
    // 1) 找到是哪一个按钮
    auto *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;

    QString loc;
    if      (btn == buttonL) loc = "left";
    else if (btn == buttonR) loc = "right";
    else return;

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

    // 2) 弹出「确认上传」对话框
    CustomMessageBox dlg(this,
                         tr("确认上传"),
                         tr("是否上传已选择的位置？"),
                         { tr("确定"), tr("取消") },
                         350*scaleY);
    dlg.exec();


    // 3) 如果用户点“返回”，直接把按钮恢复到默认样式并 return
    if (dlg.getUserResponse() != tr("确定")) {
        // 恢复原始蓝色样式
        btn->setStyleSheet(R"(
            QPushButton {
              background-color: rgba(33,150,243,0.85);
              color: #F0F0F0;
              font-weight: 600;
              font-size: 16px;
              border: none;
              border-radius: 8px;
              padding: 10px 24px;
            }
        )");
        return;
    }

    // 4) 用户点“确定”，先把按钮变成半透明绿
    btn->setStyleSheet(R"(
        QPushButton {
          background-color: rgba(76,175,80,0.85);
          color: #F0F0F0;
          font-weight: 600;
          font-size: 16px;
          border: none;
          border-radius: 8px;
          padding: 10px 24px;
        }
    )");

    // 5) 调用上传函数
    if (uploadLocation(loc)) {
        CustomMessageBox ok(this,
                            tr("成功"),
                            tr("位置已上传：%1")
                                .arg(loc=="left"?tr("左侧"):tr("右侧")),
                            { tr("确定") },
                            300*scaleY);
        ok.exec();
    }
    // 清除遮罩和模糊
    this->setGraphicsEffect(nullptr);
    overlay->close();
    overlay->deleteLater();
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

void ImplantationSite::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);
    if (event->type() == QEvent::LanguageChange) {
        // 系统自动发送的 LanguageChange
        titleLabel->setText(tr("新植入物"));
        implantTitle->setText(tr("请点击按钮选择植入位置"));
        buttonL->setText(tr("左"));
        buttonR->setText(tr("右"));
        labelCurve->setText(tr("实时波形监测"));
        helpButton->setText(tr("帮助"));
        heartTitle->setText(tr("心率监测"));
        heartUnit->setText(tr("次/分钟"));
        pressureTitle->setText(tr("血压监测"));
        signalTitle->setText(tr("信号强度"));
        returnButton->setText(tr("返回上级"));
        calibrateButton->setText(tr("校准数据"));

    }
}
