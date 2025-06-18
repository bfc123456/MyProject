#include "implantmonitor.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPixmap>
#include <qwt_plot_curve.h>
#include <QStackedLayout>
#include <QTimer>
#include <QGraphicsBlurEffect>
#include "circularprogressbar.h"
#include "rhcinputdialog.h"
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include "databasemanager.h"


ImplantMonitor::ImplantMonitor(QWidget *parent , const QString &sensorId) : FramelessWindow(parent) , m_serial(sensorId) {

    this->setFixedSize(1024,600);
    this->setObjectName("Implantonitor");
    this->setStyleSheet(R"(
    QWidget#Implantonitor {
    background-color: qlineargradient(
        x1: 0, y1: 1,
        x2: 1, y2: 0,
        stop: 0 rgba(6, 15, 30, 255),      /* 更暗靛蓝：左下 */
        stop: 0.5 rgba(18, 35, 65, 255),   /* 中段冷蓝 */
        stop: 1 rgba(30, 60, 100, 255)     /* 右上：深蓝灰 */
    );
    }

    )");

    //设置全局布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(30, 15, 30, 15);

    //设置顶部布局（放置标题与设置按钮）
    QWidget *topwidget = new QWidget(this);
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
    topwidget->setFixedHeight(50);

    titleLabel = new QLabel(tr("新植入物"));
    titleLabel->setStyleSheet("background-color: transparent; color: white;");
    idLabel = new QLabel();
    idLabel ->setText(m_serial);
    idLabel->setStyleSheet("background-color: transparent; color: white;");
    titleLabel->setFixedWidth(120);
    titleLabel->setAlignment(Qt::AlignCenter);
    idLabel->setFixedWidth(120);
    idLabel->setAlignment(Qt::AlignCenter);

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

    connect(btnSettings, &QPushButton::clicked, this, &ImplantMonitor::OpenSettingsRequested);
    QHBoxLayout *topLayout = new QHBoxLayout(topwidget);
    topLayout->addWidget(titleLabel, 0, Qt::AlignCenter);
    topLayout->addStretch();
    topLayout->addWidget(idLabel, 0, Qt::AlignCenter);
    topLayout->addStretch();
    topLayout->addWidget(btnSettings);

    //设置中间布局
    QWidget *middlewidget = new QWidget(this);
    middlewidget->setObjectName("middlewidget");
    middlewidget->setFixedHeight(350);
    middlewidget->setStyleSheet(R"(
    QWidget#middlewidget {
        background-color: qlineargradient(
            x1: 0, y1: 0, x2: 0, y2: 1,
            stop: 0 rgba(35, 45, 65, 240),   /* 顶部略亮蓝灰 */
            stop: 1 rgba(20, 30, 50, 240)    /* 底部更暗 */
        );
        border: 1px solid rgba(255, 255, 255, 0.08);      /* 柔和外边框 */
        border-radius: 12px;
        padding: 8px;
    }
    )");

    plot = new ModernWavePlot(this);

    plot->setAutoFillBackground(false);
    plot->setStyleSheet("background: transparent; border: none; color:white;");

//    // 设置曲线数据
    QVector<QPointF> points;
    for (int i = 0; i < 200; ++i)
        points.append(QPointF(i, qrand() % 81));  // 生成0到80之间的随机数
    plot->setData(points);

    // 设置样式
    plot->setLineColor(QColor(100, 180, 255));  //曲线颜色
    plot->setFillColor(QColor(40, 120, 200, 30), -1);   //波形底部填充
    plot->setYRange(0, 80); // Y 轴范围
    plot->setXRange(0, 180); //X轴范围
    plot->setMinimumHeight(200);                    // 控件高度（生效）

    //布局plot
    QVBoxLayout *middleLayout = new QVBoxLayout(middlewidget);
    middleLayout->addWidget(plot);
    middleLayout->setContentsMargins(50, 30, 50, 30);

    //设置底部布局
    QHBoxLayout *bottomLayout = new QHBoxLayout();
    //从左到右依次排布四个widget
    //左一：圆形进度显示
    QWidget *firstLeftWidget = new QWidget;
    firstLeftWidget->setFixedHeight(120);
    QVBoxLayout *firstLeftLayout = new QVBoxLayout(firstLeftWidget);
    //定义圆形进度条
//    QLabel *tittleLabel = new QLabel("信号强度",this);
//    tittleLabel->setAlignment(Qt::AlignCenter);

    //设置图形比例布局
    CircularProgressBar *progressBar = new CircularProgressBar(this);
    progressBar->setFixedSize(65, 65); // 缩小为 70x70
    progressBar->setProgress(90); // 设置进度 90%
//    firstLeftLayout->addWidget(tittleLabel);
    firstLeftLayout->addWidget(progressBar, 0, Qt::AlignCenter);

    //左二：植入位置显示
    QWidget *secondLeftWidget = new QWidget(parent);
    secondLeftWidget->setObjectName("secondLeftWidget");
    secondLeftWidget->setFixedSize(180, 120);
    secondLeftWidget->setStyleSheet(R"(
        QWidget#secondLeftWidget {
            border: none;
            /* 用 border-image 拉伸背景 */
            border-image: url(:/image/newbody.png);
        }
    )");
    //在它上面放一个 QLabel 来显示“L”或“R”
    QLabel *sideLabel = new QLabel(secondLeftWidget);
    sideLabel->setFixedSize(40, 40);
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
        sideLabel->setText("L");
        // 左侧居中偏左
        sideLabel->move( 40, 2*(secondLeftWidget->height() - sideLabel->height())/3 );
    } else {
        sideLabel->setText("R");
        // 右侧居中偏右
        sideLabel->move(
                    secondLeftWidget->width() - sideLabel->width() - 40,
                    2*(secondLeftWidget->height() - sideLabel->height())/3
                    );
    }
    sideLabel->show();

    //右二：数据显示区域
    QWidget *secondRightWidget = new QWidget;
    secondRightWidget->setFixedSize(260,120);
    QHBoxLayout *secondRightLayout = new QHBoxLayout(secondRightWidget);
    //定义控件
    bpVal = new QLabel(tr("血压\n120/80"));
    bpVal->setFixedSize(110,45);
    avgVal = new QLabel(tr("平均\n94"));
    avgVal->setFixedSize(110,45);
    hrVal = new QLabel(tr("心率\n75"));
    hrVal->setFixedSize(110,45);
    statisticsbtn = new QPushButton(tr("读数记录"));
    statisticsbtn->setFixedSize(110,45);

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

    QVBoxLayout *firstDataLayout = new QVBoxLayout;
    firstDataLayout->addWidget(bpVal, 0, Qt::AlignCenter);
    firstDataLayout->addWidget(avgVal, 0, Qt::AlignCenter);
//    firstDataLayout->addSpacing(15);
    QVBoxLayout *secondDataLayout = new QVBoxLayout;
    secondDataLayout->addWidget(hrVal, 0, Qt::AlignCenter);
    secondDataLayout->addWidget(statisticsbtn, 0, Qt::AlignCenter);
    secondRightLayout->addLayout(firstDataLayout, Qt::AlignVCenter);
    secondRightLayout->addLayout(secondDataLayout);

    //右一：按钮显示区域
    QWidget *firstRightWidget = new QWidget;
    firstRightWidget->setFixedHeight(120);
    QHBoxLayout *btntotalLayout = new QHBoxLayout(firstRightWidget);
    //定义按钮
    startBtn = new QPushButton(tr("开始测量"));
    startBtn->setFixedSize(150,45);
    inputCO = new QPushButton(tr("输入心输出量"));
    inputCO->setFixedSize(150,45);
    inputRHC = new QPushButton(tr("输入RHC"));
    inputRHC->setFixedSize(150,45);
    statBtn = new QPushButton(tr("审计界面"));

    //将控件放入布局
    QVBoxLayout *btnLiftLayout = new QVBoxLayout;
    btnLiftLayout->addWidget(inputCO);
    btnLiftLayout->addSpacing(10);//添加空白区域
    btnLiftLayout->addWidget(inputRHC);
    QVBoxLayout *btnRRightLayout = new QVBoxLayout;
    btnRRightLayout->addWidget(startBtn);
    btnRRightLayout->addSpacing(10);
    btnRRightLayout->addWidget(statBtn);
    btntotalLayout->addLayout(btnLiftLayout);
    btntotalLayout->addLayout(btnRRightLayout);

    //连接信号与槽
    connect(inputCO, &QPushButton::clicked, this, &ImplantMonitor::openCOClicked);
    connect(inputRHC, &QPushButton::clicked, this, &ImplantMonitor::openRHCClicked);
    connect(startBtn, &QPushButton::clicked, this, &ImplantMonitor::openMeasureClicked);
//    connect(startBtn, &QPushButton::clicked, this, &ImplantMonitor::startMeasurement);
    connect(statBtn, &QPushButton::clicked, this, &ImplantMonitor::openReviewClicked);

    //放置布局
    bottomLayout->addSpacing(15);
    bottomLayout->addWidget(firstLeftWidget);
    bottomLayout->addSpacing(15);
    bottomLayout->addWidget(secondLeftWidget);
    bottomLayout->addSpacing(15);
    bottomLayout->addWidget(secondRightWidget);
    bottomLayout->addSpacing(15);
    bottomLayout->addWidget(firstRightWidget);
    bottomLayout->addSpacing(15);

    mainLayout->addWidget(topwidget);
    mainLayout->addWidget(middlewidget);
    mainLayout->addLayout(bottomLayout);

    QString darkCardStyle = R"(
    QWidget {
    background-color: qlineargradient(
        x1: 0, y1: 0, x2: 0, y2: 1,
        stop: 0 rgba(25, 50, 75, 0.9),
        stop: 1 rgba(10, 20, 30, 0.85)
    );
    border-radius: 10px;
    border: 1px solid rgba(255, 255, 255, 0.08); /* 边缘高光 */
}
    )";

    firstLeftWidget->setStyleSheet(darkCardStyle);
//    secondLeftWidget->setStyleSheet(darkCardStyle);
    secondRightWidget->setStyleSheet(darkCardStyle);
    firstRightWidget->setStyleSheet(darkCardStyle);

    middlewidget->setStyleSheet(darkCardStyle);

    statBtn->setStyleSheet(R"(
        QPushButton {
            background-color: rgba(255, 255, 255, 0.08);
            color: white;
            font-size: 14px;
            font-weight: bold;
            border: 1px solid rgba(255,255,255,0.15);
            border-radius: 8px;
            padding: 8px 20px;
        }
        QPushButton:pressed {
            background-color: rgba(255, 255, 255, 0.2);
        }
    )");

    statBtn->setFixedSize(150,45);
    startBtn->setStyleSheet(R"(
        QPushButton {
            background-color: qlineargradient(
                x1: 0, y1: 0, x2: 0, y2: 1,
                stop: 0 #66FF66,
                stop: 1 #33CC33
            );
            color: white;
            font-size: 16px;
            font-weight: bold;
            border: 2px solid #228822;
            border-radius: 10px;
            padding: 2px 5px;
        }
        QPushButton:pressed {
            background-color: #2EA836;
            padding-top: 12px;
            padding-bottom: 8px;
        }
    )");

    QString secondaryBtnStyle = R"(
        QPushButton {
            background-color: qlineargradient(
                x1: 0, y1: 0, x2: 0, y2: 1,
                stop: 0 rgba(180, 180, 255, 40),
                stop: 1 rgba(120, 130, 200, 30)
            );
            color: white;
            font-size: 14px;
            border: 1px solid rgba(255, 255, 255, 40);
            border-radius: 8px;
            padding: 2px 5px;
        }
        QPushButton:pressed {
            background-color: rgba(100, 120, 200, 50);
        }
    )";
    inputCO->setStyleSheet(secondaryBtnStyle);
    inputRHC->setStyleSheet(secondaryBtnStyle);
    statBtn->setStyleSheet(secondaryBtnStyle);
    statisticsbtn->setStyleSheet(secondaryBtnStyle);
    
    connect(statisticsbtn, &QPushButton::clicked,this,&ImplantMonitor::onReadoutButtonClicked);
}

ImplantMonitor::~ImplantMonitor(){

}

void ImplantMonitor::openCOClicked()
{

    //添加遮罩层
    QWidget *overlay = new QWidget(this);
//    overlay->setGeometry(this->rect());
    overlay->setStyleSheet("background-color: rgba(0, 0, 0, 100);"); // 可调透明度
    overlay->setAttribute(Qt::WA_TransparentForMouseEvents, false); // 拦截事件
    overlay->show();
    overlay->raise();

    //添加模糊效果
    QGraphicsBlurEffect *blur = new QGraphicsBlurEffect;
    blur->setBlurRadius(20);  // 可调强度：20~40
    this->setGraphicsEffect(blur);

    CardiacOutputDialog *cardiacoutputdialog = new CardiacOutputDialog("15","0",this);
    cardiacoutputdialog->setAttribute(Qt::WA_DeleteOnClose);       // 自动销毁

    // 监听关闭信号
    connect(cardiacoutputdialog, &QDialog::finished, this, [=]() {
        this->setGraphicsEffect(nullptr);
        overlay->close();
        overlay->deleteLater();
    });

    cardiacoutputdialog->show();
}

void ImplantMonitor::openRHCClicked()
{
    //添加遮罩层
    QWidget *overlay = new QWidget(this);
//    overlay->setGeometry(this->rect());
    overlay->setStyleSheet("background-color: rgba(0, 0, 0, 100);"); // 可调透明度
    overlay->setAttribute(Qt::WA_TransparentForMouseEvents, false); // 拦截事件
    overlay->show();
    overlay->raise();

    //添加模糊效果
    QGraphicsBlurEffect *blur = new QGraphicsBlurEffect;
    blur->setBlurRadius(20);  // 可调强度：20~40
    this->setGraphicsEffect(blur);

    RHCInputDialog *dialog = new RHCInputDialog(this);
//    dialog->setAttribute(Qt::WA_DeleteOnClose);       // 自动销毁

    // dialog->setWindowModality(Qt::WindowModal);    //  不要设置

    // 监听关闭信号
    connect(dialog, &QDialog::finished, this, [=]() {
        this->setGraphicsEffect(nullptr);
        overlay->close();
        overlay->deleteLater();
    });

    dialog->show();
}


void ImplantMonitor::openMeasureClicked()
{
   measurewidget = new MeasureWidget(nullptr,m_serial);
   connect(measurewidget, &MeasureWidget::dataSaved,this,&ImplantMonitor::onDataSaved);
   measurewidget->setWindowFlags(Qt::Window);
   measurewidget->setAttribute(Qt::WA_DeleteOnClose);
   measurewidget->setFixedSize(1024,600);
   measurewidget->show();
   this->hide();
   if(measurewidget)
   connect(measurewidget,&MeasureWidget::returnprepage,[this](){
   this->show();
   });
}

void ImplantMonitor::openReviewClicked()
{
   if (!reviewwidget) {
   reviewwidget = new ReviewWidget(nullptr,m_serial);
   connect(reviewwidget,&ReviewWidget::returnToImplantmonitor,this,[this](){
       reviewwidget->setDataList(measurementList);
       reviewwidget->hide();
       this->show();
       reviewwidget->close();
   });}

   reviewwidget->setDataList(measurementList);
   reviewwidget->setFixedSize(1024,600);
   reviewwidget->show();
   this->hide();
}

//void ImplantMonitor::openReadoutClicked(){
//    readoutdialog = new ReadoutRecordDialog(this);
//    readoutdialog->setWindowFlags(Qt::Window);
//    readoutdialog->setAttribute(Qt::WA_DeleteOnClose);
//    readoutdialog->show();
//}

void ImplantMonitor::OpenSettingsRequested() {
    settingswidget = new SettingsWidget();
    settingswidget->setWindowFlags(Qt::Dialog);
    settingswidget->setAttribute(Qt::WA_DeleteOnClose);
    settingswidget->show();
}

//处理量测打包的数据
void ImplantMonitor::onDataSaved(const MeasurementData &d0) {
    MeasurementData d = d0;
    d.order = measurementList.size() + 1;    // 顺序号
    measurementList.append(d);
}

void ImplantMonitor::onReadoutButtonClicked() {
    if (!readoutdialog) {
        readoutdialog = new ReadoutRecordDialog(this);
         readoutdialog->setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
        // 这里把整个列表一次性传给对话框
        connect(this, &ImplantMonitor::dataListUpdated,readoutdialog, &ReadoutRecordDialog::populateData);
        connect(readoutdialog, &ReadoutRecordDialog::rowDeleted,this, &ImplantMonitor::onRowDeleted);
        connect(readoutdialog,&ReadoutRecordDialog::onRefreshButtonClicked,this,[this](){
           qDebug()<<"返回植入位置界面";
           measurementList.clear();
           emit returnImplantationsite();
           this->close();
        });
    }
    // 触发对话框更新
    emit dataListUpdated(measurementList);
    readoutdialog->show();
}

//删除结构体中对应的数据
void ImplantMonitor::onRowDeleted(int row)
{
    if (row < 0 || row >= measurementList.size())
        return;

    // 从容器里删掉这一条
    measurementList.removeAt(row);

    // 重新给每条数据排 order
    for (int i = 0; i < measurementList.size(); ++i)
        measurementList[i].order = i + 1;

    // 推送最新列表，让对话框刷新
    emit dataListUpdated(measurementList);
}

void ImplantMonitor::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);
    if (event->type() == QEvent::LanguageChange) {
        // 系统自动发送的 LanguageChange
        titleLabel->setText(tr("新植入物"));
        bpVal->setText(tr("血压\n120/80"));
        avgVal->setText(tr("平均\n94"));
        hrVal->setText(tr("心率\n75"));
        statisticsbtn->setText(tr("读数记录"));
        startBtn->setText(tr("开始测量"));
        inputCO->setText(tr("输入心输出量"));
        inputRHC->setText(tr("输入RHC"));
        statBtn->setText(tr("审计界面"));
    }
}

