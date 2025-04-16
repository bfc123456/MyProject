#include "reviewwidget.h"
#include <QHeaderView>
#include <QPixmap>
#include <QPen>
#include <qwt_scale_widget.h>
#include <qwt_plot_grid.h>
#include <qwt_scale_widget.h>
#include <QGraphicsBlurEffect>
#include <QDialog>
#include <QApplication>
#include "Global.h"

ReviewWidget::ReviewWidget(QWidget *parent) : QWidget(parent) {

    this->setObjectName("ReviewWidget");
    this->setStyleSheet(R"(
    #ReviewWidget {
        background-color: qlineargradient(
            x1: 0, y1: 1,
            x2: 1, y2: 0,
            stop: 0 rgba(6, 15, 30, 255),      /* 更暗靛蓝：左下 */
            stop: 0.5 rgba(18, 35, 65, 255),   /* 中段冷蓝 */
            stop: 1 rgba(30, 60, 100, 255)     /* 右上：深蓝灰 */
        );
    }
    )");


    setWindowTitle(tr("新植入物审查界面"));
    setFixedSize(1024, 600);

    //创建主布局
    QVBoxLayout *mainlayout= new QVBoxLayout(this);
    mainlayout->setContentsMargins(25,15,25,15);
    // 顶部栏
    QWidget *topBar = new QWidget(this);
    topBar->setObjectName("TopBar");
    topBar->setStyleSheet(R"(
        #TopBar {
            background-color: qlineargradient(
                x1: 0, y1: 0, x2: 0, y2: 1,
                stop: 0 rgba(25, 50, 75, 0.9),
                stop: 1 rgba(10, 20, 30, 0.85)
            );
            border-radius: 10px;
            border: 1px solid rgba(255, 255, 255, 0.08);
        }
    )");

    topBar->setFixedHeight(50);

    QLabel *titleLabel = new QLabel(tr("新植入物"), this);
    titleLabel->setFixedSize(160, 35);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: white;");
    titleLabel->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);

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
    connect(btnSettings, &QPushButton::clicked, this, &ReviewWidget::OpenSettingsRequested);

    QHBoxLayout *topLayout = new QHBoxLayout(topBar);
    topLayout->addWidget(titleLabel);
    topLayout->addStretch();
    topLayout->addWidget(btnSettings);
    topLayout->setContentsMargins(10, 0, 10, 0);
    mainlayout->addWidget(topBar);
    mainlayout->addSpacing(10);

//    QVBoxLayout *informationlayout = new QVBoxLayout();

//    // 血压监测
//    QLabel *basicinformationlabel = new QLabel("基本信息：");
//    basicinformationlabel->setStyleSheet("color: white; font-weight: bold;");
//    QLabel *sensornumberlabel = new QLabel("序列号");
//    sensornumberlabel->setStyleSheet("color: white; font-weight: bold;");
//    informationlayout->addSpacing(5);
//    informationlayout->addWidget(basicinformationlabel);
//    informationlayout->addWidget(sensornumberlabel);
//    informationlayout->addSpacing(5);
//    informationlayout->setContentsMargins(30,0,30,0);
//    mainlayout->addLayout(informationlayout);

    //中间行左侧：血压监测部分
    QHBoxLayout *middlelayout = new QHBoxLayout();
    QWidget *middleliftwidget = new QWidget();
    middleliftwidget->setFixedSize(470,220);
    middleliftwidget->setObjectName("chartview");
    middleliftwidget->setStyleSheet(R"(
        #chartview {
        background-color: rgba(41, 41, 41, 230);  /* 深灰磨砂背景 */
        border: 2px solid #1E90FF;                /* 亮蓝边框 */
        border-radius: 15px;                      /* 圆角 */
        }
    )");

    QVBoxLayout *middleliftlayout = new QVBoxLayout(middleliftwidget);

    QLabel *monitorpressure = new QLabel(tr("血压监测"));
    monitorpressure->setStyleSheet("color: white;font-size: 15px;");
    middleliftlayout->addWidget(monitorpressure);

    QGridLayout *bpGrid = new QGridLayout();
    // 第一行：传感器
    QLabel *sensorTitle = new QLabel(tr("传感器"));
    QLabel *sensorValue = new QLabel("<b>142/86</b>");
    QLabel *sensorUnit = new QLabel("mmHg");
    QLabel *sensorPulse = new QLabel("93");

    // 第二行：参考值
    QLabel *refTitle = new QLabel(tr("参考值"));
    QLabel *refValue = new QLabel("<b>142/86</b>");
    QLabel *refUnit = new QLabel("mmHg");
    QLabel *refPulse = new QLabel("93");

    //设置文本颜色
    sensorTitle->setStyleSheet("color: white; font-size: 13px;");
    sensorValue->setStyleSheet("color: white;font-size: 15px;");
    sensorUnit->setStyleSheet("color: white; font-size: 13px;");
    sensorPulse->setStyleSheet("color: white;font-size: 15px;");

    refTitle->setStyleSheet("color: white; font-size: 13px;");
    refValue->setStyleSheet("color: white;font-size: 15px;");
    refUnit->setStyleSheet("color: white; font-size: 13px;");
    refPulse->setStyleSheet("color: white;font-size: 15px;");

    // 添加到网格（行, 列）
    bpGrid->addWidget(sensorTitle, 0, 0);
    bpGrid->addWidget(sensorValue, 0, 1);
    bpGrid->addWidget(sensorUnit, 0, 2);
    bpGrid->addWidget(sensorPulse, 0, 3);

    bpGrid->addWidget(refTitle, 1, 0);
    bpGrid->addWidget(refValue, 1, 1);
    bpGrid->addWidget(refUnit, 1, 2);
    bpGrid->addWidget(refPulse, 1, 3);
    bpGrid->setContentsMargins(90,0,90,0);
    middleliftlayout->addLayout(bpGrid,Qt::AlignCenter);

    /**************************波线图的刻度颜色待修正********************************************/
    //波形图区
    QwtPlot *bpPlot = new QwtPlot();
    bpPlot->setCanvasBackground(QColor(31, 31, 31));  // 深灰色背景

    // 设置刻度范围
    bpPlot->setAxisScale(QwtPlot::yLeft, 0, 150, 30);
    bpPlot->setAxisScale(QwtPlot::xBottom, 0, 4, 1);

    // 创建刻度字体
    QFont axisFont("Arial", 10);

    // 设置灰色字体颜色
    QPalette axisPalette;
    axisPalette.setColor(QPalette::WindowText, QColor(180, 180, 180));   // 设置刻度值颜色
    axisPalette.setColor(QPalette::Foreground, QColor(180, 180, 180));   // 设置刻度线颜色（必要时）

    // 应用于 Y 轴
    QwtScaleWidget *yAxis = bpPlot->axisWidget(QwtPlot::yLeft);
    yAxis->setFont(axisFont);
    yAxis->setPalette(axisPalette);

    // 应用于 X 轴
    QwtScaleWidget *xAxis = bpPlot->axisWidget(QwtPlot::xBottom);
    xAxis->setFont(axisFont);
    xAxis->setPalette(axisPalette);

    // 曲线设置
    QwtPlotCurve *bpCurve = new QwtPlotCurve("血压曲线");

    QwtPlotGrid *grid = new QwtPlotGrid();
    grid->setMajorPen(QPen(QColor(90, 90, 90), 1, Qt::DashLine));  // 主网格线样式
    grid->enableX(true);   // 启用 X 方向主刻度线参考线
    grid->enableY(true);   // 启用Y轴参考线
    grid->attach(bpPlot);  // 绑定到 QwtPlot

    bpCurve->setSamples(QVector<double>{0, 1, 2, 3, 4}, QVector<double>{0, 30, 60, 120, 150});
    bpCurve->setPen(QPen(QColor(0, 255, 255), 2));  // 青色曲线
    bpCurve->setBrush(QBrush(QColor(0, 180, 255, 40)));  // 填充淡蓝色
    bpCurve->setRenderHint(QwtPlotItem::RenderAntialiased, true);
    bpCurve->attach(bpPlot);
    bpPlot->setMinimumHeight(100);
    bpPlot->replot();

    middleliftlayout->addSpacing(15);
    middleliftlayout->addWidget(bpPlot);
    middlelayout->addWidget(middleliftwidget);
    middlelayout->addSpacing(10);

    // 中间右侧：心率监测部分
    QWidget *middlerightwidget = new QWidget();
    middlerightwidget->setFixedSize(470,220);
    middlerightwidget->setObjectName("HeartContainer");
    middlerightwidget->setStyleSheet(R"(
        #HeartContainer {
            background-color: rgba(41, 41, 41, 230);  /* 深灰磨砂背景 */
            border: 2px solid #1E90FF;                /* 亮蓝边框 */
            border-radius: 15px;                      /* 圆角 */
        }
    )");
    QHBoxLayout *middlerightlayout = new QHBoxLayout(middlerightwidget);
    QVBoxLayout *heartratetittlelayout = new QVBoxLayout();
    heartratetittlelayout->setContentsMargins(50,10,50,10);
    heartratetittlelayout->addSpacing(50);
    QLabel *hrLabel = new QLabel(tr("心率监测"));
    hrLabel->setStyleSheet("color: white; font-size: 15px; font-weight: bold;");
    QLabel *hrValue = new QLabel("78 BPM");
    hrValue->setStyleSheet("color: white; font-size: 15px; font-weight: bold;");
    heartratetittlelayout->addWidget(hrLabel);
    heartratetittlelayout->addWidget(hrValue);
    heartratetittlelayout->addSpacing(50);
    middlerightlayout->addLayout(heartratetittlelayout);

    QLabel *heartImageLabel = new QLabel();
    heartImageLabel->setPixmap(QPixmap(":/image/body.png").scaled(500, 180, Qt::KeepAspectRatio));
    middlerightlayout->addWidget(heartImageLabel);
    middlerightlayout->addSpacing(15);

    middlelayout->addWidget(middlerightwidget);
    middlelayout->setContentsMargins(15,0,15,0);
    mainlayout->addLayout(middlelayout);

    // 历史记录
    QHBoxLayout *historylayout = new QHBoxLayout();
    QWidget *historycheckwidget = new QWidget();

    historycheckwidget->setObjectName("HistoryCheck");
    historycheckwidget->setStyleSheet(R"(
        #HistoryCheck {
            background-color: rgba(41, 41, 41, 230);  /* 深灰磨砂背景 */
            border: 2px solid #1E90FF;                /* 亮蓝边框 */
            border-radius: 15px;                      /* 圆角 */
        }
    )");

    QTableWidget *historyTable = new QTableWidget(1, 4);

    historyTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    historyTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    historyTable->setHorizontalHeaderLabels(QStringList() << tr("序号") << tr("传感器") << tr("参考值") << tr("心率"));
    historyTable->setItem(0, 0, new QTableWidgetItem("1"));
    historyTable->setItem(0, 1, new QTableWidgetItem("142/86 (94)"));
    historyTable->setItem(0, 2, new QTableWidgetItem("134/86 (92)"));
    historyTable->setItem(0, 3, new QTableWidgetItem("78"));
    historyTable->horizontalHeader()->setStretchLastSection(true);
    historyTable->setMinimumHeight(120);
    historyTable->verticalHeader()->setStyleSheet(R"(
        QHeaderView {
            background-color: #1f1f1f;
            color: white;
            border: none;
        }
        QHeaderView::section {
            background-color: #1f1f1f;
            color: white;
            border: none;
            padding: 4px;
        }
    )");

    historyTable->setStyleSheet(R"(
        QTableWidget {
            background-color: #1f1f1f;
            color: white;
            border: none;
            font-size: 14px;
            gridline-color: #3e3e3e;
            selection-background-color: #2e8bff;
            selection-color: white;
        }

        QHeaderView::section {
            background-color: #2b2b2b;
            color: white;
            font-weight: bold;
            padding: 6px;
            border: 1px solid #3e3e3e;
        }

        QHeaderView::section:vertical {
            background-color: #2b2b2b;
            color: white;
            border: 1px solid #3e3e3e;
        }

        QTableCornerButton::section {
            background-color: #2b2b2b;
            border: 1px solid #3e3e3e;
        }

        QScrollBar:vertical {
            background: transparent;
            width: 10px;
            margin: 0px;
        }

        QScrollBar::handle:vertical {
            background: #3a8dff;
            border-radius: 5px;
            min-height: 20px;
        }

        QScrollBar::add-line:vertical,
        QScrollBar::sub-line:vertical {
            height: 0px;
        }

        QTableView::item {
            padding: 6px;
        }
    )");




    QLabel *historylabel = new QLabel(tr("历史记录"),historycheckwidget);
    historylabel->setStyleSheet("color: white;");
    QVBoxLayout *historychecklayout = new QVBoxLayout(historycheckwidget);
    historychecklayout->addWidget(historylabel);
    historychecklayout->addWidget(historyTable);
    historylayout->addSpacing(15);
    historylayout->addWidget(historycheckwidget);
    historylayout->addSpacing(15);


    // 按钮
    QPushButton *backButton = new QPushButton(tr("返回"));
    backButton->setFixedSize(120, 35);
    backButton->setIcon(QIcon(":/image/icons8-return.png"));
    QPushButton *saveButton = new QPushButton(tr("保存并返回"));
    saveButton->setFixedSize(120, 35);
    saveButton->setIcon(QIcon(":/image/icons8-save.png"));
    backButton->setStyleSheet(R"(
        QPushButton {
            background-color: qlineargradient(
                x1:0, y1:0, x2:0, y2:1,
                stop:0 rgba(95, 169, 246, 180),
                stop:1 rgba(49, 122, 198, 180)
            );
            border: 1px solid rgba(163, 211, 255, 0.6);
            border-radius: 6px;
            color: white;
            font-weight: bold;
            font-size: 14px;
            padding: 6px 12px;

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

    saveButton->setStyleSheet(R"(
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
            padding: 6px 12px;
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

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(backButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(saveButton);
    buttonLayout->setContentsMargins(15,0,15,0);

    connect(saveButton,&QPushButton::clicked,this,[this](){
        showExitConfirmWidget();
    });

    // 主体布局
    mainlayout->addSpacing(10);
    mainlayout->addLayout(historylayout);
    mainlayout->addSpacing(10);
    mainlayout->addLayout(buttonLayout);
}

ReviewWidget::~ReviewWidget(){

}

void ReviewWidget::showExitConfirmWidget()
{
    qDebug() << "展示退出确认界面信号已接收";

    // 添加遮罩层
    QWidget *overlay = new QWidget(this);
    overlay->setGeometry(this->rect());
    overlay->setStyleSheet("background-color: rgba(0, 0, 0, 100);");  // 可调透明度
    overlay->setAttribute(Qt::WA_TransparentForMouseEvents, false); // 拦截事件
    overlay->show();
    overlay->raise();

    // 添加模糊效果
    QGraphicsBlurEffect *blur = new QGraphicsBlurEffect;
    blur->setBlurRadius(20);  // 可调强度：20~40
    this->setGraphicsEffect(blur);

    // 创建提示弹窗
    QDialog prompt(this);
    prompt.setStyleSheet(R"(
        QDialog, QWidget {
            background-color: qlineargradient(
                x1: 0, y1: 0, x2: 0, y2: 1,
                stop: 0 rgba(30, 50, 80, 0.9),
                stop: 1 rgba(10, 25, 50, 0.75)
            );
            color: white;
            font-size: 14px;
            border-radius: 10px;
        }
    )");
    prompt.setWindowTitle(tr("提示"));
    prompt.setFixedSize(400, 200);

    // 内容布局与按钮
    QVBoxLayout *mainLayout = new QVBoxLayout(&prompt);

    QLabel *label = new QLabel(tr("请确认您的下一步操作"));
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet("font-size: 16px;");
    mainLayout->addWidget(label);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    QPushButton *exitButton = new QPushButton(tr("关闭系统"));
    QPushButton *returnButton = new QPushButton(tr("返回主界面"));

    // 按钮样式略...
    exitButton->setFixedSize(120, 45);
    exitButton->setIcon(QIcon(":/image/icons8-shutdown.png"));
    returnButton->setFixedSize(120, 45);
    returnButton->setIcon(QIcon(":/image/icons8-return.png"));

    exitButton->setStyleSheet(R"(
        QPushButton {
            background-color: qlineargradient(
                x1:0, y1:0, x2:0, y2:1,
                stop:0 rgba(95, 169, 246, 180),
                stop:1 rgba(49, 122, 198, 180)
            );
            border: 1px solid rgba(163, 211, 255, 0.6);
            border-radius: 6px;
            color: white;
            font-weight: bold;
            font-size: 14px;
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

    returnButton->setStyleSheet(R"(
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

    buttonLayout->addStretch();
    buttonLayout->addWidget(exitButton);
    buttonLayout->addSpacing(40);
    buttonLayout->addWidget(returnButton);
    buttonLayout->addStretch();

    mainLayout->addLayout(buttonLayout);

    QObject::connect(exitButton, &QPushButton::clicked, QApplication::instance(), &QApplication::quit);
    QObject::connect(returnButton, &QPushButton::clicked, this, [this, &prompt]() {
         this->hide();  // 隐藏回顾界面
         if (globalLoginWindowPointer) {
             globalLoginWindowPointer->show();  // 显示主界面
         }
         this->close();  // 关闭当前回顾界面
         prompt.close();
//         delete this;    // 删除当前窗口对象，释放内存
     });

    // 显示对话框
    prompt.exec();

    // 清除遮罩和模糊
    this->setGraphicsEffect(nullptr);
    overlay->close();
    overlay->deleteLater();
}

void ReviewWidget::OpenSettingsRequested() {
    settingswidget = new SettingsWidget();
    settingswidget->setWindowFlags(Qt::Dialog);
    settingswidget->setAttribute(Qt::WA_DeleteOnClose);
    settingswidget->show();
}


