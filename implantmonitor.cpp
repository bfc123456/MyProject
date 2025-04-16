#include "implantmonitor.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPixmap>
#include <qwt_plot_curve.h>
#include <QStackedLayout>
#include <QTimer>
#include <QGraphicsBlurEffect>
#include "CircularProgressBar.h"
#include "rhcinputdialog.h"
#include "measurewidget.h"
#include "reviewwidget.h"
#include "modernwaveplot.h"
#include "readoutrecorddialog.h"

ImplantMonitor::ImplantMonitor(QWidget *parent) : QWidget(parent) {

    this->setFixedSize(1024,600);
    this->setObjectName("Implantonitor");
    this->setStyleSheet(R"(
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
        #TopBar {
            background-color: qlineargradient(
                x1: 0, y1: 0, x2: 0, y2: 1,
                stop: 0 rgba(25, 50, 75, 0.9),
                stop: 1 rgba(10, 20, 30, 0.85)
            );
            border-radius: 10px;
            border: 1px solid rgba(255, 255, 255, 0.08); /* 边缘高光 */
        }
    )");
    topwidget->setFixedHeight(50);

    titleLabel = new QLabel(tr("新植入物"));
    titleLabel->setStyleSheet("background-color: transparent; color: white; font-size: 16px;");
    idLabel = new QLabel("XXXXXXXX");
    idLabel->setStyleSheet("background-color: transparent; color: white; font-size: 16px;");
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

        /* 模拟内阴影：靠边暗线条 */
        box-shadow: inset 0 1px 2px rgba(255, 255, 255, 0.06),
                    0 2px 6px rgba(0, 0, 0, 0.3);         /* 外部阴影效果 */
    }
    )");

    ModernWavePlot *plot = new ModernWavePlot(this);

    plot->setAutoFillBackground(false);
    plot->setStyleSheet("background: transparent; border: none;");

    // 设置曲线数据
    QVector<QPointF> points;
    for (int i = 0; i < 200; ++i)
        points.append(QPointF(i, qrand() % 50 + 60));
    plot->setData(points);

    // 设置样式
    plot->setLineColor(QColor(100, 180, 255));  //曲线颜色
    plot->setFillColor(QColor(40, 120, 200, 30), 20);   //波形底部填充
    plot->setYRange(60, 110);                       // Y 轴范围
    plot->setMinimumHeight(200);                    // 控件高度（生效）

    //布局plot
    QVBoxLayout *middleLayout = new QVBoxLayout(middlewidget);
    middleLayout->addWidget(plot);
    middleLayout->setContentsMargins(50, 30, 50, 30);

    //设置底部布局
    QHBoxLayout *bottomLayout = new QHBoxLayout(this);
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
    QWidget *secondLeftWidget = new QWidget;
    imgLabel = new QLabel(secondLeftWidget);
    imgLabel->setFixedSize(180, 120);
    QPixmap original(":/image/body.png");
    QPixmap scaled = original.scaled(imgLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    imgLabel->setPixmap(scaled);
    imgLabel->setAlignment(Qt::AlignCenter);
    secondLeftWidget->setFixedSize(180, 120);  // 容器控件尺寸固定

    // 左右半透明框
    QFrame* leftRegion = new QFrame(imgLabel);
    QFrame* rightRegion = new QFrame(imgLabel);

    // 设置初始大小和位置：左右各一半
    leftRegion->setGeometry(0, 0, 90, 120);   // 左半边
    rightRegion->setGeometry(90, 0, 90, 120); // 右半边

    auto setImplantSide = [&](bool isLeft){
        if (isLeft) {
            leftRegion->setStyleSheet("background-color: rgba(0, 200, 0, 80);");
            rightRegion->setStyleSheet("background-color: rgba(200, 200, 200, 40);");
        } else {
            leftRegion->setStyleSheet("background-color: rgba(200, 200, 200, 40);");
            rightRegion->setStyleSheet("background-color: rgba(0, 200, 0, 80);");
        }
    };
    setImplantSide(true);

//    centerLayout->addStretch();

    //右二：数据显示区域
    QWidget *secondRightWidget = new QWidget;
    secondRightWidget->setFixedSize(260,120);
    QHBoxLayout *secondRightLayout = new QHBoxLayout(secondRightWidget);
    //定义控件
    bpVal = new QLabel("血压\n120/80");
    bpVal->setFixedSize(110,45);
    avgVal = new QLabel("平均\n94");
    avgVal->setFixedSize(110,45);
    hrVal = new QLabel("心率\n75");
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
            padding: 6px;
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
    secondLeftWidget->setStyleSheet(darkCardStyle);
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
            padding: 10px 24px;
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
            padding: 4px 10px;
        }
        QPushButton:pressed {
            background-color: rgba(100, 120, 200, 50);
        }
    )";
    inputCO->setStyleSheet(secondaryBtnStyle);
    inputRHC->setStyleSheet(secondaryBtnStyle);
    statBtn->setStyleSheet(secondaryBtnStyle);
    statisticsbtn->setStyleSheet(secondaryBtnStyle);
    
    connect(statisticsbtn,&QPushButton::clicked,this,&ImplantMonitor::openReadoutClicked);

    setLayout(mainLayout);
}

ImplantMonitor::~ImplantMonitor(){

}

void ImplantMonitor::openCOClicked()
{

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
    overlay->setGeometry(this->rect());
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
   qDebug()<<"test";
   MeasureWidget *measurewidget = new MeasureWidget(this);
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
   qDebug()<<"test";
   ReviewWidget *reviewwidget = new ReviewWidget();
   reviewwidget->setFixedSize(1024,600);
   reviewwidget->show();
   this->hide();
}

void ImplantMonitor::openReadoutClicked(){
    ReadoutRecordDialog *readoutdialog = new ReadoutRecordDialog(this);
    readoutdialog->setWindowFlags(Qt::Window);
    readoutdialog->setAttribute(Qt::WA_DeleteOnClose);
    readoutdialog->show();
}

void ImplantMonitor::OpenSettingsRequested() {
    settingswidget = new SettingsWidget();
    settingswidget->setWindowFlags(Qt::Dialog);
    settingswidget->setAttribute(Qt::WA_DeleteOnClose);
    settingswidget->show();
}
