#include "measurewidget.h"
#include <qwt_plot_grid.h>
#include <qwt_series_data.h>
#include "circularprogressbar.h"
#include <QGridLayout>
#include <qwt_scale_widget.h>
#include <qwt_plot_layout.h>
#include <QGraphicsBlurEffect>
#include <QDialog>
#include <QTimer>
#include "CustomMessageBox.h"

MeasureWidget::MeasureWidget(QWidget *parent , const QString &sensorId)
    : FramelessWindow(parent), m_serial(sensorId)
{

    QVBoxLayout *mainlayout = new QVBoxLayout(this);
    mainlayout->setContentsMargins(30, 15, 30, 15);
    this->setObjectName("Measurewidget");
    this->setStyleSheet(R"(
    QWidget#Measurewidget {
        background-color: qlineargradient(
            x1: 0, y1: 1,
            x2: 1, y2: 0,
            stop: 0 rgba(6, 15, 30, 255),      /* 更暗靛蓝：左下 */
            stop: 0.5 rgba(18, 35, 65, 255),   /* 中段冷蓝 */
            stop: 1 rgba(30, 60, 100, 255)     /* 右上：深蓝灰 */
        );
    }
    )");


    //设置顶部信息状态栏
    QWidget *topwidget = new QWidget(this);
    topwidget->setObjectName("topwidget");
    topwidget->setStyleSheet(R"(
        QWidget#topwidget {
            background-color: qlineargradient(
                x1: 0, y1: 0, x2: 0, y2: 1,
                stop: 0 rgba(25, 50, 75, 0.9),
                stop: 1 rgba(10, 20, 30, 0.85)
            );
            border-radius: 10px;
            border: 1px solid rgba(255, 255, 255, 0.08);
        }
         QLabel {
             color: white;
             font-weight: bold;
             font-size: 16px;
         }
    )");

    topwidget->setFixedHeight(50);

    //状态栏标题
    titleLabel = new QLabel(tr("新植入物"));
    titleLabel->setStyleSheet("background-color: transparent; color: white;");
    idLabel = new QLabel();
    idLabel->setText(m_serial);
    idLabel->setStyleSheet("background-color: transparent; color: white;");
    titleLabel->setFixedWidth(120);
    titleLabel->setAlignment(Qt::AlignCenter);
    idLabel->setFixedWidth(120);
    idLabel->setAlignment(Qt::AlignCenter);

    //设置按键
    QPushButton *btnSettings = new QPushButton(this);
    btnSettings->setIcon(QIcon(":/image/icons8-shezhi.png"));
    btnSettings->setIconSize(QSize(24, 24));
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
    connect(btnSettings, &QPushButton::clicked, this, &MeasureWidget::OpenSettingsRequested);

    QHBoxLayout *topLayout = new QHBoxLayout(topwidget);
    topLayout->addWidget(titleLabel, 0, Qt::AlignCenter);
    topLayout->addStretch();
    topLayout->addWidget(idLabel, 0, Qt::AlignCenter);
    topLayout->addStretch();
    topLayout->addWidget(btnSettings);
    mainlayout->addWidget(topwidget);
    mainlayout->addSpacing(10);
    //设置中间左侧布局
    QHBoxLayout *middleLayout = new QHBoxLayout();
    QWidget *middleliftwidget = new QWidget(this);
    middleliftwidget->setObjectName("middleliftwidget");
    middleliftwidget->setStyleSheet(R"(
    background-color: qlineargradient(
        x1: 0, y1: 0, x2: 0, y2: 1,
        stop: 0 rgba(40, 55, 80, 230),
        stop: 1 rgba(25, 35, 55, 230)
    );
    border: 1px solid rgba(255, 255, 255, 0.06);
    border-radius: 10px;

    )");
    QHBoxLayout *middleliftLayout = new QHBoxLayout(middleliftwidget);
    middleliftwidget->setFixedSize(250,250);
    middleliftLayout->setAlignment(Qt::AlignCenter);

    //设置图形比例布局
    CircularProgressBar *progressBar = new CircularProgressBar(middleliftwidget);
    progressBar->setFixedSize(120, 120); // 缩小为 70x70
    progressBar->setProgress(90); // 设置进度 90%
    middleliftLayout->addWidget(progressBar,Qt::AlignCenter);

    //设置中间右侧布局
    QWidget *middlerightwidget = new QWidget(this);
    middlerightwidget->setObjectName("middlerightwidget");
    middlerightwidget->setStyleSheet(R"(
    QWidget #middlerightwidget{
     background-color: qlineargradient(
         x1: 0, y1: 0, x2: 0, y2: 1,
         stop: 0 rgba(40, 55, 80, 230),
         stop: 1 rgba(25, 35, 55, 230)
     );
     border: 1px solid rgba(255, 255, 255, 0.06);
     border-radius: 10px;
     }
    )");
    middlerightwidget->setFixedSize(700,250);
    QHBoxLayout *middlerightLayout = new QHBoxLayout(middlerightwidget);

    // 创建实例
    plot = new ModernWavePlot(this);

    plot->setAutoFillBackground(false);
    plot->setStyleSheet("background: transparent; border: none;");

    // 配置曲线数据
//    QVector<QPointF> data;
//    for (int i = 0; i < 200; ++i)
//        data << QPointF(i, 40 + 20 * std::sin(i * 0.1));

    // 设置样式和数据
    plot->setFixedSize(800, 235);
    plot->setYRange(0, 80); // Y 轴范围
    plot->setXRange(0, 180); //X轴范围
    plot->setLineColor(QColor(100, 180, 255));
    plot->setFillColor(QColor(40, 120, 200, 30),  -1);
    plot->setCanvasBackground(QColor(20, 30, 50, 230));
//    plot->setData(data);


    middlerightLayout->addWidget(plot,Qt::AlignCenter);
//    middlerightLayout->setContentsMargins(15,5,15,5);

    middleLayout->addWidget(middleliftwidget);
    middleLayout->addSpacing(10);
    middleLayout->addWidget(middlerightwidget);
    mainlayout->addLayout(middleLayout);
    mainlayout->addSpacing(10);

    //创建底部布局
    QHBoxLayout *bottomlayout = new QHBoxLayout ();
    //底部布局
    QWidget *bottomliftwidget = new QWidget(this);
    bottomliftwidget->setObjectName("bottomliftwidget");
    bottomliftwidget->setStyleSheet(R"(
    QWidget #bottomliftwidget{
    background-color: qlineargradient(
        x1: 0, y1: 0, x2: 0, y2: 1,
        stop: 0 rgba(40, 55, 80, 230),
        stop: 1 rgba(25, 35, 55, 230)
    );
    border: 1px solid rgba(255, 255, 255, 0.06);
    border-radius: 10px;
    }
    )");
    QVBoxLayout *bottomliftlayout = new QVBoxLayout (bottomliftwidget);

    //标题标签
    sensormonitor = new QLabel(tr("传感器检测数据"));
    sensormonitor->setStyleSheet("background-color: transparent; color: #7DDFFF; font-size: 18px;");
    bottomliftlayout->addWidget(sensormonitor);

    QHBoxLayout *realdataLayout = new QHBoxLayout();

    //收缩压
    QVBoxLayout *sbplayout = new QVBoxLayout();
    sbptittlle = new QLabel(tr("收缩压"));
    sbptittlle->setStyleSheet("background-color: transparent; color: white; font-size: 14px;");
    sbplayout->addWidget(sbptittlle);
    QHBoxLayout *sbpdatalayout = new QHBoxLayout();
    sbpdata = new QLabel("120");
    sbpdata->setStyleSheet("background-color: transparent; color: white; font-size: 16px;");
    QLabel *sbpunit = new QLabel("mmHg");
    sbpunit->setStyleSheet("background-color: transparent; color: white; font-size: 16px;");
    sbpdatalayout->addWidget(sbpdata);
    sbpdatalayout->addWidget(sbpunit);
    sbplayout->addLayout(sbpdatalayout);

    //舒张压
    QVBoxLayout *dbplayout = new QVBoxLayout();
    dbptittlle = new QLabel(tr("舒张压"));
    dbptittlle->setStyleSheet("background-color: transparent; color: white; font-size: 14px;");
    dbplayout->addWidget(dbptittlle);
    QHBoxLayout *dbpdatalayout = new QHBoxLayout();
    dbpdata = new QLabel("80");
    dbpdata->setStyleSheet("background-color: transparent; color: white; font-size: 16px;");
    QLabel *dbpunit = new QLabel("mmHg");
    dbpunit->setStyleSheet("background-color: transparent; color: white; font-size: 16px;");
    dbpdatalayout->addWidget(dbpdata);
    dbpdatalayout->addWidget(dbpunit);
    dbplayout->addLayout(dbpdatalayout);

    //平均值
    QVBoxLayout *meanlayout = new QVBoxLayout();
    meantittlle = new QLabel(tr("平均"));
    meantittlle->setStyleSheet("background-color: transparent; color: white; font-size: 14px;");
    meanlayout->addWidget(meantittlle);
    QHBoxLayout *meandatalayout = new QHBoxLayout();
    meandata = new QLabel("80");
    meandata->setStyleSheet("background-color: transparent; color: white; font-size: 16px;");
    QLabel *meanunit = new QLabel("mmHg");
    meanunit->setStyleSheet("background-color: transparent; color: white; font-size: 16px;");
    meandatalayout->addWidget(meandata);
    meandatalayout->addWidget(meanunit);
    meanlayout->addLayout(meandatalayout);

    //心率
    QVBoxLayout *heartratelayout = new QVBoxLayout();
    heartratetittlle = new QLabel(tr("心率"));
    heartratetittlle->setStyleSheet("background-color: transparent; color: white; font-size: 14px;");
    heartratelayout->addWidget(heartratetittlle);
    QHBoxLayout *heartrateunitlayout = new QHBoxLayout();
    heartratedata = new QLabel("80");
    heartratedata->setStyleSheet("background-color: transparent; color: white; font-size: 16px;");
    QLabel *heartrateunit = new QLabel("mmHg");
    heartrateunit->setStyleSheet("background-color: transparent; color: white; font-size: 16px;");
    heartrateunitlayout->addWidget(heartratedata);
    heartrateunitlayout->addWidget(heartrateunit);
    heartratelayout->addLayout(heartrateunitlayout);
    
    //放置四个参数布局到实时数据布局
    realdataLayout->addLayout(sbplayout);
    realdataLayout->addLayout(dbplayout);
    realdataLayout->addLayout(meanlayout);
    realdataLayout->addLayout(heartratelayout);
    bottomliftlayout->addLayout(realdataLayout);
    bottomliftlayout->addSpacing(10);
    referencetittle = new QLabel(tr("参考值"));
    referencetittle->setStyleSheet("background-color: transparent; color: #7DDFFF; font-size: 18px;");
    bottomliftlayout->addWidget(referencetittle);
    
    //设置参数数值区域布局
    QHBoxLayout *referencedatalayout = new QHBoxLayout();
    
    QVBoxLayout *refersbpdatalayout = new QVBoxLayout();
    
    //参考收缩压
    const QString modernLineEditStyle = R"(
        QLineEdit {
            background-color: rgba(255, 255, 255, 0.05);
            border: 1px solid rgba(255, 255, 255, 0.08);
            border-radius: 8px;
            padding: 6px 10px;
            color: #ffffff;
            font-size: 13px;
            selection-background-color: rgba(100, 200, 255, 0.3);
        }
        QLineEdit:focus {
            border: 1px solid rgba(100, 200, 255, 0.4);
            background-color: rgba(255, 255, 255, 0.08);
        }
    )";

    refersbptittlle = new QLabel(tr("收缩压"));
    refersbptittlle->setStyleSheet("background-color: transparent; color: white; font-size: 14px;");
    refersbpdatalayout->addWidget(refersbptittlle);
    QHBoxLayout *refersbpinputlayout = new QHBoxLayout();
    referinputsbpdata = new QLineEdit();
    referinputsbpdata->setPlaceholderText(tr("请输入参考收缩压..."));
    referinputsbpdata->setFixedHeight(30);
    referinputsbpdata->setStyleSheet(modernLineEditStyle);
    QLabel * referinputsbpunit = new QLabel("mmHg");
    referinputsbpunit->setStyleSheet("background-color: transparent; color: white; font-size: 16px;");
    refersbpinputlayout->addWidget(referinputsbpdata);
    refersbpinputlayout->addWidget(referinputsbpunit);
    refersbpdatalayout->addLayout(refersbpinputlayout);
 
    //参考舒张压
    QVBoxLayout *referdbdatalayout = new QVBoxLayout();
    referdbptittlle = new QLabel(tr("舒张压"));
    referdbptittlle->setStyleSheet("background-color: transparent; color: white; font-size: 14px;");
    referdbdatalayout->addWidget(referdbptittlle);
    QHBoxLayout *referdbpinputlayout = new QHBoxLayout();
    referinputdbpdata = new QLineEdit();
    referinputdbpdata->setPlaceholderText(tr("请输入参考舒张压..."));
    referinputdbpdata->setFixedHeight(30);
    referinputdbpdata->setStyleSheet(modernLineEditStyle);
    QLabel * referinputdbpunit = new QLabel("mmHg");
    referinputdbpunit->setStyleSheet("background-color: transparent; color: white; font-size: 16px;");
    referdbpinputlayout->addWidget(referinputdbpdata);
    referdbpinputlayout->addWidget(referinputdbpunit);
    referdbdatalayout->addLayout(referdbpinputlayout);
    
    //参考平均值
    QVBoxLayout *refermeandatalayout = new QVBoxLayout();
    refermeantittlle = new QLabel(tr("平均"));
    refermeantittlle->setStyleSheet("background-color: transparent; color: white; font-size: 14px;");
    refermeandatalayout->addWidget(refermeantittlle);
    QHBoxLayout *refermeaninputlayout = new QHBoxLayout();
    refermeaninputdata = new QLineEdit();
    refermeaninputdata->setPlaceholderText(tr("请输入参考平均值..."));
    refermeaninputdata->setFixedHeight(30);
    refermeaninputdata->setStyleSheet(modernLineEditStyle);
    QLabel * refermeaninputunit = new QLabel("mmHg");
    refermeaninputunit->setStyleSheet("background-color: transparent; color: white; font-size: 16px;");
    refermeaninputlayout->addWidget(refermeaninputdata);
    refermeaninputlayout->addWidget(refermeaninputunit);
    refermeandatalayout->addLayout(refermeaninputlayout);
    
    //放置参考数值布局
    referencedatalayout->addLayout(refersbpdatalayout);
    referencedatalayout->addLayout(referdbdatalayout);
    referencedatalayout->addLayout(refermeandatalayout);
    bottomliftlayout->addLayout(referencedatalayout);
    
    //设置第三行右侧布局
    QWidget *bottomrightwidget = new QWidget();
    bottomrightwidget->setObjectName("bottomrightwidget");
    bottomrightwidget->setStyleSheet(R"(
    QWidget #bottomrightwidget{
    background-color: qlineargradient(
        x1: 0, y1: 0, x2: 0, y2: 1,
        stop: 0 rgba(40, 55, 80, 230),
        stop: 1 rgba(25, 35, 55, 230)
    );
    border: 1px solid rgba(255, 255, 255, 0.06);
    border-radius: 10px;
    }
    )");
    QVBoxLayout *bottomrightlayout = new QVBoxLayout(bottomrightwidget);
    positontittle = new QLabel(tr("患者位置调整"));
    positontittle->setStyleSheet("background-color: transparent; color: #7DDFFF; font-size: 16px;");
    bottomrightlayout->addWidget(positontittle);


    QGridLayout *positionbuttonlayout = new QGridLayout();
    zerobutton = new QPushButton("0°");
    zerobutton->setStyleSheet("QPushButton {"
                              "background-color:rgba(135, 206, 235, 0.6);"  // 恢复为蓝色背景
                              "color: white;"  // 恢复白色文字
                              "font-size: 14px;"
                              "font-weight: bold;"
                              "border: 1px solid rgba(120, 200, 255, 0.3);"
                              "border-radius: 8px;"
                              "padding: 6px 12px;"
                              "}");
    thirtybutton = new QPushButton("30°");
    thirtybutton->setStyleSheet("QPushButton {"
                                "background-color:rgba(135, 206, 235, 0.6);"  // 恢复为蓝色背景
                                "color: white;"  // 恢复白色文字
                                "font-size: 14px;"
                                "font-weight: bold;"
                                "border: 1px solid rgba(120, 200, 255, 0.3);"
                                "border-radius: 8px;"
                                "padding: 6px 12px;"
                                "}");
    fortyfivebutton = new QPushButton("45°");
    fortyfivebutton->setStyleSheet("QPushButton {"
                                   "background-color:rgba(135, 206, 235, 0.6);"  // 恢复为蓝色背景
                                   "color: white;"  // 恢复白色文字
                                   "font-size: 14px;"
                                   "font-weight: bold;"
                                   "border: 1px solid rgba(120, 200, 255, 0.3);"
                                   "border-radius: 8px;"
                                   "padding: 6px 12px;"
                                   "}");
    ninetybutton = new QPushButton("90°");
    ninetybutton->setStyleSheet("QPushButton {"
                                "background-color:rgba(135, 206, 235, 0.6);"  // 恢复为蓝色背景
                                "color: white;"  // 恢复白色文字
                                "font-size: 14px;"
                                "font-weight: bold;"
                                "border: 1px solid rgba(120, 200, 255, 0.3);"
                                "border-radius: 8px;"
                                "padding: 6px 12px;"
                                "}");

    //关联函数
    // 在构造函数或者合适的位置连接按钮点击信号
    connect(zerobutton, &QPushButton::clicked, this, &MeasureWidget::onBtnClicked);
    connect(thirtybutton, &QPushButton::clicked, this, &MeasureWidget::onBtnClicked);
    connect(fortyfivebutton, &QPushButton::clicked, this, &MeasureWidget::onBtnClicked);
    connect(ninetybutton, &QPushButton::clicked, this, &MeasureWidget::onBtnClicked);

    positionbuttonlayout->addWidget(zerobutton,0,0);
    positionbuttonlayout->addWidget(thirtybutton,0,1);
    positionbuttonlayout->addWidget(fortyfivebutton,1,0);
    positionbuttonlayout->addWidget(ninetybutton,1,1);
    bottomrightlayout->addLayout(positionbuttonlayout);
    
    remarklabel = new QLabel(tr("备注"));
    remarklabel->setStyleSheet("background-color: transparent; color: #7DDFFF; font-size: 16px;");
    remarklineedit = new QLineEdit();
    remarklineedit->setFixedHeight(50);
    remarklineedit->setPlaceholderText(tr("请输入备注信息..."));
    remarklineedit->setStyleSheet(modernLineEditStyle);
    bottomrightlayout->addWidget(remarklabel);
    bottomrightlayout->addWidget(remarklineedit);


    // 拿到单例键盘
    currentKeyboard = CustomKeyboard::instance(this);

    // 给每个 QLineEdit 注册一次偏移（如果你想要默认偏移都一样，就写同一个 QPoint）
    currentKeyboard->registerEdit(referinputsbpdata, QPoint(259,-300));
    currentKeyboard->registerEdit(referinputdbpdata, QPoint(70,-300));
    currentKeyboard->registerEdit(refermeaninputdata, QPoint(-119,-300));
    currentKeyboard->registerEdit(remarklineedit, QPoint(-351,-300));

    bottomlayout->addWidget(bottomliftwidget);
    bottomlayout->addSpacing(25);
    bottomlayout->addWidget(bottomrightwidget);

    //放置主布局
    mainlayout->addLayout(bottomlayout);
    mainlayout->addSpacing(10);
    
    //创建保存按钮
    QHBoxLayout *savebtnlayout = new QHBoxLayout();
    savebtn = new QPushButton(tr("保存"));
    savebtn->setFixedSize(120,35);
    connect(savebtn,&QPushButton::clicked,this,&MeasureWidget::openSaveConfirm);
    savebtn->setStyleSheet(R"(
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

    returnbtn = new QPushButton(tr("返回"));
    returnbtn->setFixedSize(120,35);
    connect(returnbtn,&QPushButton::clicked,this,&MeasureWidget::returnImplantmonitor);
    returnbtn->setStyleSheet(R"(
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

    savebtnlayout->addWidget(returnbtn);
    savebtnlayout->addStretch();
    savebtnlayout->addWidget(savebtn);

    mainlayout->addLayout(savebtnlayout);
    mainlayout->addSpacing(5);

    startMeasurement();
}

MeasureWidget::~MeasureWidget() {
    // 析构函数
}

void MeasureWidget::OpenSettingsRequested() {
    settingswidget = new SettingsWidget();
    settingswidget->setWindowFlags(Qt::Dialog);
    settingswidget->setAttribute(Qt::WA_DeleteOnClose);
    settingswidget->show();
}

void MeasureWidget::returnImplantmonitor() {
     emit returnprepage();
     this->hide();
     this->close();
}

void MeasureWidget::openSaveConfirm(){

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

    //创建提示弹窗
    CustomMessageBox dlg(
        this,
        tr("提示"),
        tr("数据保存成功"),
        { tr("确 认") },
        400      // 对话框宽度
    );

    int result = dlg.exec();

    //根据用户点击结果来做后续处理
    if (result == QDialog::Accepted) {
        // 执行保存后的逻辑
        onSaveButtonClicked();

        // 关闭当前界面并通知上一级
        this->close();
        emit returnprepage();
    }

    // 清除遮罩和模糊
    this->setGraphicsEffect(nullptr);
    overlay->close();
    overlay->deleteLater();

}

    //开始生成波形数据
void MeasureWidget::startMeasurement() {
    // 初始化开始时间
    currentTime = 0;
    points.clear();  // 清空之前的数据

    // 设置定时器，每秒触发一次timeout信号
    measurementTimer = new QTimer(this);
    connect(measurementTimer, &QTimer::timeout, this, &MeasureWidget::updateWaveform);

    // 启动定时器，每1000毫秒（1秒）触发一次
    measurementTimer->start(1000);
}

void MeasureWidget::updateWaveform() {
    // 每秒增加一个数据点
    points.append(QPointF(currentTime, qrand() % 81)); // 生成0到80之间的随机数，模拟波形数据

    // 计算最大值和最小值
    maxValue = points[0].y();
    minValue = points[0].y();
    avgValue = 0;
    heartRate = 0;

    for (const auto& point : points) {
        if (point.y() > maxValue) {
            maxValue = point.y();  // 更新最大值
        }
        if (point.y() < minValue) {
            minValue = point.y();  // 更新最小值
        }
    }

    // 计算平均值
    double sum = 0;
    for (const auto &point : points) {
        sum += point.y();
    }
    avgValue = sum / points.size();

    // 计算心率
    heartRate = calculateHeartRate();


    // 更新血压标签的显示内容（格式：血压\n最大值/最小值）
    sbpdata->setText(QString("%1").arg(QString::number(maxValue, 'f', 2)));
    dbpdata->setText(QString("%1").arg(QString::number(minValue, 'f', 2)));
    meandata->setText(QString("%1").arg(QString::number(heartRate, 'f', 2)));
    heartratedata->setText(QString("%1").arg(QString::number(avgValue, 'f', 2)));

    // 设置新的波形数据到plot
    plot->setData(points);

    // 更新图表
    plot->replot();

    // 增加时间
    currentTime++;

    // 假设当时间超过180秒时停止
    if (currentTime > 180) {
        measurementTimer->stop(); // 停止定时器
    }
}

    //简单的心率计算
double MeasureWidget::calculateHeartRate() {
    int peaks = 0;
    for (int i = 1; i < points.size() - 1; ++i) {
        if (points[i].y() > points[i - 1].y() && points[i].y() > points[i + 1].y()) {
            peaks++;
        }
    }
    return peaks * 60.0 / points.size();  // 假设每个峰值代表一个心跳，计算心率
}

    //位置按钮点击反馈函数
void MeasureWidget::onBtnClicked() {
    QPushButton *clickedBtn = qobject_cast<QPushButton*>(sender());

    if (!isYellow) {
        clickedBtn->setStyleSheet("QPushButton {"
                                  "background-color: rgba(0, 255, 0, 0.6);"  // 设置淡绿色并调整透明度
                                  "color: black;"  // 设置文字颜色为黑色
                                  "font-size: 14px;"
                                  "font-weight: bold;"
                                  "border: 1px solid rgba(120, 200, 255, 0.3);"  // 设置边框的颜色和样式
                                  "border-radius: 8px;"  // 设置圆角边框
                                  "padding: 6px 12px;"
                                  "}");
        isYellow = true; // 标记按钮已经变为黄色

        selectedAngle  = clickedBtn->text();

    } else {
        clickedBtn->setStyleSheet("QPushButton {"
                                  "background-color:rgba(135, 206, 235, 0.6);"  // 恢复为蓝色背景
                                  "color: white;"  // 恢复白色文字
                                  "font-size: 14px;"
                                  "font-weight: bold;"
                                  "border: 1px solid rgba(120, 200, 255, 0.3);"
                                  "border-radius: 8px;"
                                  "padding: 6px 12px;"
                                  "}");
        isYellow = false; // 标记按钮恢复为初始状态

        selectedAngle.clear();
    }
}

void MeasureWidget::onSaveButtonClicked() {

    MeasurementData d;

    //取值
    d.sensorSystolic   = sbpdata->text();
    d.sensorDiastolic  = dbpdata->text();
    d.sensorAvg        = meandata->text();
    d.heartRate        = heartratedata->text();
    d.refSystolic      = referinputsbpdata->text();
    d.refDiastolic     = referinputdbpdata->text();
    d.refAvg           = refermeaninputdata->text();
    d.angle            = selectedAngle;
    d.note             = remarklineedit->text();
    d.points           = this->points;

    // 发射信号，传递所有数据
    emit dataSaved(d);
}

void MeasureWidget::changeEvent(QEvent *event){
    QWidget::changeEvent(event);
    if (event->type() == QEvent::LanguageChange) {
        // 系统自动发送的 LanguageChange
        titleLabel->setText(tr("新植入物"));
        sensormonitor->setText(tr("传感器检测数据"));
        sbptittlle->setText(tr("收缩压"));
        dbptittlle->setText(tr("舒张压"));
        meantittlle->setText(tr("平均"));
        heartratetittlle->setText(tr("心率"));
        referencetittle->setText(tr("参考值"));
        refersbptittlle->setText(tr("收缩压"));
        referinputsbpdata->setText(tr("请输入参考收缩压..."));
        referdbptittlle->setText(tr("舒张压"));
        referinputdbpdata->setText(tr("请输入参考舒张压..."));
        refermeantittlle->setText(tr("平均"));
        refermeaninputdata->setText(tr("请输入参考平均值..."));
        positontittle->setText(tr("患者位置调整"));
        remarklabel->setText(tr("备注"));
        remarklineedit->setText(tr("请输入备注信息..."));
        savebtn->setText(tr("保存"));
        returnbtn->setText(tr("返回"));

    }
}

