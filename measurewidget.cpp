#include "measurewidget.h"
#include <qwt_plot_grid.h>
#include <qwt_plot_curve.h>
#include <qwt_series_data.h>
#include "CircularProgressBar.h"
#include <QGridLayout>
#include <qwt_scale_widget.h>
#include <qwt_plot_layout.h>
#include <QGraphicsBlurEffect>
#include <QDialog>
#include "modernwaveplot.h"


MeasureWidget::MeasureWidget(QWidget *parent) : QWidget(parent)
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
    )");

    topwidget->setFixedHeight(50);

    //状态栏标题
    titleLabel = new QLabel(tr("新植入物"));
    titleLabel->setStyleSheet("background-color: transparent; color: white; font-size: 16px;");
    idLabel = new QLabel("XXXXXXXX");
    idLabel->setStyleSheet("background-color: transparent; color: white; font-size: 16px;");
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
    ModernWavePlot* plot = new ModernWavePlot(this);

    // 配置曲线数据
    QVector<QPointF> data;
    for (int i = 0; i < 200; ++i)
        data << QPointF(i, 40 + 20 * std::sin(i * 0.1));

    // 设置样式和数据
    plot->setFixedSize(800, 235);
    plot->setYRange(20, 60);
    plot->setLineColor(QColor(100, 180, 255));
    plot->setFillColor(QColor(40, 120, 200, 30), 20);
    plot->setCanvasBackground(QColor(20, 30, 50, 230));
    plot->setData(data);

    QFont axisFont("Arial", 10, QFont::Bold);
    plot->axisWidget(QwtPlot::xBottom)->setFont(axisFont);
    plot->axisWidget(QwtPlot::yLeft)->setFont(axisFont);


    middlerightLayout->addWidget(plot,Qt::AlignCenter);

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
    QLabel *sensormonitor = new QLabel(tr("传感器检测数据"));
    sensormonitor->setStyleSheet("background-color: transparent; color: #7DDFFF; font-size: 18px;");
    bottomliftlayout->addWidget(sensormonitor);

    QHBoxLayout *realdataLayout = new QHBoxLayout();

    //收缩压
    QVBoxLayout *sbplayout = new QVBoxLayout();
    QLabel *sbptittlle = new QLabel(tr("收缩压"));
    sbptittlle->setStyleSheet("background-color: transparent; color: white; font-size: 14px;");
    sbplayout->addWidget(sbptittlle);
    QHBoxLayout *sbpdatalayout = new QHBoxLayout();
    QLabel *sbpdata = new QLabel("120");
    sbpdata->setStyleSheet("background-color: transparent; color: white; font-size: 16px;");
    QLabel *sbpunit = new QLabel("mmHg");
    sbpunit->setStyleSheet("background-color: transparent; color: white; font-size: 16px;");
    sbpdatalayout->addWidget(sbpdata);
    sbpdatalayout->addWidget(sbpunit);
    sbplayout->addLayout(sbpdatalayout);

    //舒张压
    QVBoxLayout *dbplayout = new QVBoxLayout();
    QLabel *dbptittlle = new QLabel(tr("舒张压"));
    dbptittlle->setStyleSheet("background-color: transparent; color: white; font-size: 14px;");
    dbplayout->addWidget(dbptittlle);
    QHBoxLayout *dbpdatalayout = new QHBoxLayout();
    QLabel *dbpdata = new QLabel("80");
    dbpdata->setStyleSheet("background-color: transparent; color: white; font-size: 16px;");
    QLabel *dbpunit = new QLabel("mmHg");
    dbpunit->setStyleSheet("background-color: transparent; color: white; font-size: 16px;");
    dbpdatalayout->addWidget(dbpdata);
    dbpdatalayout->addWidget(dbpunit);
    dbplayout->addLayout(dbpdatalayout);

    //平均值
    QVBoxLayout *meanlayout = new QVBoxLayout();
    QLabel *meantittlle = new QLabel(tr("平均"));
    meantittlle->setStyleSheet("background-color: transparent; color: white; font-size: 14px;");
    meanlayout->addWidget(meantittlle);
    QHBoxLayout *meandatalayout = new QHBoxLayout();
    QLabel *meandata = new QLabel("80");
    meandata->setStyleSheet("background-color: transparent; color: white; font-size: 16px;");
    QLabel *meanunit = new QLabel("mmHg");
    meanunit->setStyleSheet("background-color: transparent; color: white; font-size: 16px;");
    meandatalayout->addWidget(meandata);
    meandatalayout->addWidget(meanunit);
    meanlayout->addLayout(meandatalayout);

    //心率
    QVBoxLayout *heartratelayout = new QVBoxLayout();
    QLabel *heartratetittlle = new QLabel(tr("心率"));
    heartratetittlle->setStyleSheet("background-color: transparent; color: white; font-size: 14px;");
    heartratelayout->addWidget(heartratetittlle);
    QHBoxLayout *heartrateunitlayout = new QHBoxLayout();
    QLabel *heartratedata = new QLabel("80");
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
    QLabel *referencetittle = new QLabel(tr("参考值"));
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
            font-size: 14px;
            selection-background-color: rgba(100, 200, 255, 0.3);
        }
        QLineEdit:focus {
            border: 1px solid rgba(100, 200, 255, 0.4);
            background-color: rgba(255, 255, 255, 0.08);
        }
    )";

    QLabel*refersbptittlle = new QLabel(tr("收缩压"));
    refersbptittlle->setStyleSheet("background-color: transparent; color: white; font-size: 14px;");
    refersbpdatalayout->addWidget(refersbptittlle);
    QHBoxLayout *refersbpinputlayout = new QHBoxLayout();
    QLineEdit * referinputsbpdata = new QLineEdit();
    referinputsbpdata->setFixedHeight(30);
    referinputsbpdata->setStyleSheet(modernLineEditStyle);
    QLabel * referinputsbpunit = new QLabel("mmHg");
    referinputsbpunit->setStyleSheet("background-color: transparent; color: white; font-size: 16px;");
    refersbpinputlayout->addWidget(referinputsbpdata);
    refersbpinputlayout->addWidget(referinputsbpunit);
    refersbpdatalayout->addLayout(refersbpinputlayout);
 
    //参考舒张压
    QVBoxLayout *referdbdatalayout = new QVBoxLayout();
    QLabel*referdbptittlle = new QLabel(tr("舒张压"));
    referdbptittlle->setStyleSheet("background-color: transparent; color: white; font-size: 14px;");
    referdbdatalayout->addWidget(referdbptittlle);
    QHBoxLayout *referdbpinputlayout = new QHBoxLayout();
    QLineEdit * referinputdbpdata = new QLineEdit();
    referinputdbpdata->setFixedHeight(30);
    referinputdbpdata->setStyleSheet(modernLineEditStyle);
    QLabel * referinputdbpunit = new QLabel("mmHg");
    referinputdbpunit->setStyleSheet("background-color: transparent; color: white; font-size: 16px;");
    referdbpinputlayout->addWidget(referinputdbpdata);
    referdbpinputlayout->addWidget(referinputdbpunit);
    referdbdatalayout->addLayout(referdbpinputlayout);
    
    //参考平均值
    QVBoxLayout *refermeandatalayout = new QVBoxLayout();
    QLabel*refermeantittlle = new QLabel(tr("平均"));
    refermeantittlle->setStyleSheet("background-color: transparent; color: white; font-size: 14px;");
    refermeandatalayout->addWidget(refermeantittlle);
    QHBoxLayout *refermeaninputlayout = new QHBoxLayout();
    QLineEdit * refermeaninputdata = new QLineEdit();
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
    QLabel * positontittle = new QLabel(tr("患者位置调整"));
    positontittle->setStyleSheet("background-color: transparent; color: #7DDFFF; font-size: 16px;");
    bottomrightlayout->addWidget(positontittle);
    
    const QString buttonStyle = R"(
        QPushButton {
            background-color: qlineargradient(
                x1:0, y1:0, x2:0, y2:1,
                stop:0 rgba(90, 160, 255, 0.7),
                stop:1 rgba(40, 100, 200, 0.7)
            );
            border: 1px solid rgba(120, 200, 255, 0.3);
            border-radius: 8px;
            color: white;
            font-size: 14px;
            font-weight: bold;
            padding: 6px 12px;
        }

        QPushButton:pressed {
            background-color: rgba(30, 90, 160, 0.9);
            border: 1px solid rgba(100, 200, 255, 0.6);
            padding-left: 8px;
            padding-top: 7px;
        }
    )";


    QGridLayout *positionbuttonlayout = new QGridLayout();
    QPushButton *zerobutton = new QPushButton("0°");
    zerobutton->setStyleSheet(buttonStyle);
    QPushButton *thirtybutton = new QPushButton("30°");
    thirtybutton->setStyleSheet(buttonStyle);
    QPushButton *fortyfivebutton = new QPushButton("45°");
    fortyfivebutton->setStyleSheet(buttonStyle);
    QPushButton *ninetybutton = new QPushButton("90°");
    ninetybutton->setStyleSheet(buttonStyle);
    positionbuttonlayout->addWidget(zerobutton,0,0);
    positionbuttonlayout->addWidget(thirtybutton,0,1);
    positionbuttonlayout->addWidget(fortyfivebutton,1,0);
    positionbuttonlayout->addWidget(ninetybutton,1,1);
    bottomrightlayout->addLayout(positionbuttonlayout);
    
    QLabel *remarklabel = new QLabel(tr("备注"));
    remarklabel->setStyleSheet("background-color: transparent; color: #7DDFFF; font-size: 16px;");
    QLineEdit *remarklineedit = new QLineEdit();
    remarklineedit->setFixedHeight(50);
    remarklineedit->setPlaceholderText(tr("请输入备注信息..."));
    remarklineedit->setStyleSheet(modernLineEditStyle);
    bottomrightlayout->addWidget(remarklabel);
    bottomrightlayout->addWidget(remarklineedit);
   
    bottomlayout->addWidget(bottomliftwidget);
    bottomlayout->addSpacing(25);
    bottomlayout->addWidget(bottomrightwidget);

    //放置主布局
    mainlayout->addLayout(bottomlayout);
    mainlayout->addSpacing(10);
    
    //创建保存按钮
    QHBoxLayout *savebtnlayout = new QHBoxLayout();
    QPushButton *savebtn = new QPushButton(tr("保存"));
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

    QPushButton *returnbtn = new QPushButton(tr("返回"));
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
    QDialog prompt(this);
    prompt.setStyleSheet(R"(
        QDialog {
            background-color: qlineargradient(
                x1: 0, y1: 0, x2: 0, y2: 1,
                stop: 0 rgba(15, 34, 67, 200),     /* 深蓝：顶部 */
                stop: 1 rgba(10, 25, 50, 180)      /* 更深蓝：底部 */
            );
            border-radius: 12px;
        }

        QLabel {
            color: rgba(255, 255, 255, 230);
            font-size: 16px;
            font-weight: bold;
            background: transparent;   /* 关键！避免白底 */
        }
    )");

    prompt.setWindowTitle(tr("提示"));
    prompt.setFixedSize(400, 200);

    //内容布局与按钮
    QVBoxLayout* mainLayout = new QVBoxLayout(&prompt);

    QLabel* label = new QLabel(tr("数据保存成功"));
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet("font-size: 16px;");
    mainLayout->addWidget(label);

    QHBoxLayout* buttonLayout = new QHBoxLayout;
    QPushButton* confirmButton = new QPushButton(tr("确 认"));
    confirmButton->setFixedSize(120,45);
    confirmButton->setStyleSheet(R"(
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

    buttonLayout->addWidget(confirmButton,Qt::AlignCenter);

    mainLayout->addLayout(buttonLayout);

    QObject::connect(confirmButton, &QPushButton::clicked, &prompt, &QDialog::accept);

    // 5. 阻塞显示
    prompt.exec();

    // 6. 清除遮罩和模糊
    this->setGraphicsEffect(nullptr);
    overlay->close();
    overlay->deleteLater();

}
