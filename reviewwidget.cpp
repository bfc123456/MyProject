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
#include "global.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QGuiApplication>
#include <QScreen>

ReviewWidget::ReviewWidget(QWidget *parent , const QString &sensorId)
  : FramelessWindow(parent), m_serial(sensorId) {

    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int screenWidth = screenGeometry.width();
    int screenHeight = screenGeometry.height();

    // 计算缩放比例
    scaleX = (float)screenWidth / 1024;
    scaleY = (float)screenHeight / 600;

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
    setFixedSize(1024*scaleX, 600*scaleY);

    //创建主布局
    QVBoxLayout *mainlayout= new QVBoxLayout(this);
    mainlayout->setContentsMargins(25*scaleX,15*scaleY,25*scaleX,15*scaleY);
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
          QLabel {
              color: white;
              font-weight: bold;
              font-size: 16px;
          }
    )");

    topBar->setFixedHeight(50*scaleY);

    titleLabel = new QLabel(tr("新植入物"), this);
    titleLabel->setFixedSize(120*scaleX, 35*scaleY);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: white;");
    titleLabel->setAlignment(Qt::AlignCenter);

    idLabel = new QLabel();
    idLabel->setText(m_serial);
    idLabel->setStyleSheet("background-color: transparent; color: white;");
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
    connect(btnSettings, &QPushButton::clicked, this, &ReviewWidget::OpenSettingsRequested);

    QHBoxLayout *topLayout = new QHBoxLayout(topBar);
    topLayout->setContentsMargins(10*scaleX, 0, 10*scaleX, 0);
    topLayout->addWidget(titleLabel, 0, Qt::AlignLeft);
    topLayout->addWidget(idLabel, 1, Qt::AlignCenter);
    topLayout->addSpacing(96*scaleX);
    topLayout->addWidget(btnSettings, 0, Qt::AlignRight);
    mainlayout->addWidget(topBar);
    mainlayout->addSpacing(10*scaleY);

    //中间行左侧：血压监测部分
    QHBoxLayout *middlelayout = new QHBoxLayout();
    QWidget *middleliftwidget = new QWidget();
    middleliftwidget->setFixedSize(470*scaleX,220*scaleY);
    middleliftwidget->setObjectName("chartview");
    middleliftwidget->setStyleSheet(R"(
        #chartview {
        background-color: rgba(41, 41, 41, 230);  /* 深灰磨砂背景 */
        border: 2px solid #1E90FF;                /* 亮蓝边框 */
        border-radius: 15px;                      /* 圆角 */
        }
    )");

    QVBoxLayout *middleliftlayout = new QVBoxLayout(middleliftwidget);
    middleliftlayout->addSpacing(10*scaleY);
    monitorpressure = new QLabel(tr("血压监测"));
    monitorpressure->setStyleSheet("color: white;font-size: 15px; font-weight: bold;");
    middleliftlayout->addWidget(monitorpressure);

    /**************************波线图的刻度颜色待修正********************************************/
    //波形图区
    bpPlot = new ModernWavePlot();
    bpPlot->setAutoFillBackground(false);
    bpPlot->setStyleSheet("background: transparent; border: none; color:white;");
//    bpPlot->setYRange(0, 80); // Y 轴范围
    bpPlot->setXRange(0, 180); //X轴范围
    bpPlot->setAxisScale(QwtPlot::yLeft, 0.0, 80.0, 20.0);

    bpPlot->setLineColor(QColor(100, 180, 255));  //曲线颜色
    bpPlot->setFillColor(QColor(40, 120, 200, 30), -1);   //波形底部填充

    middleliftlayout->addSpacing(15*scaleY);
    middleliftlayout->addWidget(bpPlot);
    middlelayout->addWidget(middleliftwidget);
    middlelayout->addSpacing(10*scaleX);

    // 中间右侧：心率监测部分
    QWidget *middlerightwidget = new QWidget();
    middlerightwidget->setFixedSize(420*scaleX,220*scaleY);//-120
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
    heartratetittlelayout->setContentsMargins(50*scaleX,10*scaleY,50*scaleX,10*scaleY);
    heartratetittlelayout->addSpacing(50*scaleY);
    hrLabel = new QLabel(tr("植入位置:"));
    hrLabel->setStyleSheet("color: white; font-size: 16px; font-weight: bold;");
    QLabel *hrValue = new QLabel();
    QString loc = DatabaseManager::instance().getLocationBySensorId(m_serial).trimmed().toLower();
    hrValue->setStyleSheet("color: white; font-size: 28px; font-weight: bold;");

    // 映射数据库查询结果为中文
    if (loc == "left") {
        hrValue->setText(tr("左"));
    }
    else if (loc == "right") {
        hrValue->setText(tr("右"));
    }
    else {
        hrValue->setText(tr("未知"));
    }
    hrValue->setAlignment(Qt::AlignCenter);
    hrValue->setStyleSheet("color: white; font-size: 15px; font-weight: bold;");

    heartratetittlelayout->addWidget(hrLabel);
    heartratetittlelayout->addWidget(hrValue);
    heartratetittlelayout->addSpacing(50*scaleY);
    heartratetittlelayout->setContentsMargins(25*scaleX,0,25*scaleX,0);
    middlerightlayout->addSpacing(30*scaleX);
    middlerightlayout->addLayout(heartratetittlelayout);
    middlerightlayout->addSpacing(30*scaleX);

    // 创建父容器
    QWidget *imgContainer = new QWidget;
    imgContainer->setFixedSize(200*scaleX, 150*scaleY);

    // 底层：图片
    QLabel *lblImg = new QLabel(imgContainer);
    lblImg->setPixmap(
        QPixmap(":/image/newbody.png")
            .scaled(
                imgContainer->size(),
                Qt::KeepAspectRatio,          // 改这一行
                Qt::SmoothTransformation
            )
    );
    lblImg->setAlignment(Qt::AlignCenter);
    lblImg->setFixedSize(imgContainer->size());

    //顶层：半透明文字框
    QLabel *lblOverlay = new QLabel(hrValue->text(), imgContainer);
    lblOverlay->setAttribute(Qt::WA_TransparentForMouseEvents);
    lblOverlay->setAlignment(Qt::AlignCenter);
    lblOverlay->setStyleSheet(QString(R"(
        background-color: rgba(33,150,243,200);
        color: white;
        font-size: %1px;
        font-weight: bold;
        border-radius: 6px;
    )").arg(20*scaleY));
    lblOverlay->setFixedSize(40*scaleX, 40*scaleY);

    // 手动定位 overlay 到容器上（这里示例放在右肺区域）
    int x = imgContainer->width() * 0.7 - lblOverlay->width()/2;
    int y = imgContainer->height() * 0.6 - lblOverlay->height()/2;
    lblOverlay->move(x, y);

    //  把容器添加到布局
    middlerightlayout->addWidget(imgContainer);
    middlerightlayout->addSpacing(30*scaleX);

    middlelayout->addWidget(middlerightwidget);
    middlelayout->setContentsMargins(15*scaleX,0,15*scaleX,0);
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

    historyTable = new QTableWidget(1, 6);

    historyTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    historyTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    historyTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    historyTable->setHorizontalHeaderLabels(QStringList() << tr("传感器ID") << tr("舒张压") << tr("收缩压") << tr("平均值") << tr("心率") << tr("测量时间"));
    historyTable->horizontalHeader()->setStretchLastSection(true);
    historyTable->setMinimumHeight(120*scaleY);
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

    historylabel = new QLabel(tr("历史记录"),historycheckwidget);
    historylabel->setStyleSheet("color: white;");
    QVBoxLayout *historychecklayout = new QVBoxLayout(historycheckwidget);
    historychecklayout->addWidget(historylabel);
    historychecklayout->addWidget(historyTable);
    historylayout->addSpacing(15*scaleX);
    historylayout->addWidget(historycheckwidget);
    historylayout->addSpacing(15*scaleX);


    // 按钮
    backButton = new QPushButton(tr("返回"));
    backButton->setFixedSize(135*scaleX, 35*scaleY);
    connect(backButton,&QPushButton::clicked,this,&ReviewWidget::returnToImplantmonitor);
    backButton->setIcon(QIcon(":/image/icons8-return.png"));
    saveButton = new QPushButton(tr("保存并返回"));
    saveButton->setFixedSize(135*scaleX, 35*scaleY);
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
            padding: 2px 5px;

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
            padding: 2px 5px;
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
    buttonLayout->setContentsMargins(15*scaleX,0,15*scaleX,0);

    connect(saveButton,&QPushButton::clicked,this,[this](){
        uploadToDatabase();
        showExitConfirmWidget();
    });

    // 点击表格某行，就取出对应的波形数据画到 plot
    connect(historyTable, &QTableWidget::cellClicked, this, [=](int row,int){
        if (row >= 0 && row < m_list.size()) {
            bpPlot->setData(m_list[row].points);
            bpPlot->replot();
        }
    });

    // 主体布局
    mainlayout->addSpacing(10*scaleY);
    mainlayout->addLayout(historylayout);
    mainlayout->addSpacing(10*scaleY);
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
    prompt.setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
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
    prompt.setFixedSize(400*scaleX, 200*scaleY);

    // 内容布局与按钮
    QVBoxLayout *mainLayout = new QVBoxLayout(&prompt);

    QLabel *label = new QLabel(tr("请确认您的下一步操作"));
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet("background-color: transparent; color: white; font-size: 16px;");
    mainLayout->addWidget(label);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    QPushButton *exitButton = new QPushButton(tr("关闭系统"));
    QPushButton *returnButton = new QPushButton(tr("返回主界面"));

    // 按钮样式略...
    exitButton->setFixedSize(135*scaleX, 45*scaleY);
    exitButton->setIcon(QIcon(":/image/icons8-shutdown.png"));
    returnButton->setFixedSize(135*scaleX, 45*scaleY);
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
    buttonLayout->addSpacing(40*scaleY);
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

void ReviewWidget::setDataList(const QList<MeasurementData> &list) {

    m_list = list;

    historyTable->clearContents();
    historyTable->setRowCount(m_list.size());

    for (int row = 0; row < m_list.size(); ++row) {
        const auto &d = m_list[row];
        auto mk = [&](const QString &txt){
            auto *it = new QTableWidgetItem(txt);
            it->setTextAlignment(Qt::AlignCenter);
            return it;
        };

        historyTable->setItem(row, 0, mk(d.sensorId ));
        historyTable->setItem(row, 1, mk(d.sensorSystolic));
        historyTable->setItem(row, 2, mk(d.sensorDiastolic));
        historyTable->setItem(row, 3, mk(d.sensorAvg));
        historyTable->setItem(row, 4, mk(d.heartRate));
        historyTable->setItem(row, 5, mk(d.timestamp));
    }

    // 一打开就默认画第一行（如果有的话）
    if (!m_list.isEmpty()) {
        bpPlot->setData(m_list.first().points);
        bpPlot->replot();
    }
}

//插入数据到数据库
bool ReviewWidget::uploadToDatabase()
{
    QSqlDatabase db = QSqlDatabase::database();
    // 1) 开事务
    if (!db.transaction()) {
        qWarning() << "开启事务失败：" << db.lastError().text();
        return false;
    }

    // 2) 一次性准备好 INSERT 语句
    QSqlQuery q(db);
    q.prepare(R"(
        INSERT INTO measurements_data_update
          (sensor_id,      systolic,
           diastolic,      avg_value,
           heart_rate,     timestamp)
        VALUES
          (:sensorID,     :sys,
           :dia,          :avg,
           :hr,           :ts)
    )");

    int rows = historyTable->rowCount();
    for (int r = 0; r < rows; ++r) {
        // 从 tableWidget 里取出每一行的各列
        QString sensorID    = historyTable->item(r, 0)->text();   // 传感器读数
        QString systolic    = historyTable->item(r, 1)->text();
        QString diastolic   = historyTable->item(r, 2)->text();
        QString avgValue    = historyTable->item(r, 3)->text();
        QString heartRate   = historyTable->item(r, 4)->text();
        QString timestamp   = historyTable->item(r, 5)->text();

        // 3) 绑定值
        q.bindValue(":sensorID",    m_serial);         // 外键：序列号
        q.bindValue(":sys",         systolic);   // 新列：传感器读数
        q.bindValue(":dia",         diastolic);
        q.bindValue(":avg",         avgValue);
        q.bindValue(":hr",          heartRate);
        q.bindValue(":ts",          timestamp);

        // 4) 执行并检查
        if (!q.exec()) {
            qWarning() << "插入第" << r << "行失败：" << q.lastError().text();
            db.rollback();
            return false;
        }
    }

    // 5) 全部成功后提交
    if (!db.commit()) {
        qWarning() << "提交事务失败：" << db.lastError().text();
        db.rollback();
        return false;
    }

    qDebug() << "所有记录上传完成。";
    return true;
}

void ReviewWidget::changeEvent(QEvent *event){
    QWidget::changeEvent(event);
    if (event->type() == QEvent::LanguageChange) {
        // 系统自动发送的 LanguageChange
        titleLabel->setText(tr("新植入物"));
        monitorpressure->setText(tr("血压监测"));
        hrLabel->setText(tr("植入位置:"));
        historyTable->setHorizontalHeaderLabels(
            QStringList()
            << tr("传感器ID")
            << tr("舒张压")
            << tr("收缩压")
            << tr("平均值")
            << tr("心率")
            << tr("测量时间")
        );
        historylabel->setText(tr("历史记录"));
        backButton->setText(tr("返回"));
        saveButton->setText(tr("保存并返回"));
    }
}
