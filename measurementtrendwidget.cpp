#include "measurementtrendwidget.h"
#include <QScreen>
#include <QGuiApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QPainter>
#include <QFont>
#include <QDebug>
#include <QPainterPath>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QtGlobal>
#include <global.h>

/*************** TrendChart 类实现****************/

#include <qwt_plot_grid.h>
#include <qwt_symbol.h>
#include <qwt_curve_fitter.h>
#include <qwt_scale_widget.h>
#include <qwt_date_scale_draw.h>
#include <qwt_date_scale_engine.h>

TrendChart::TrendChart(QWidget *parent)
    : QwtPlot(parent),
      curveDia(new QwtPlotCurve(tr("舒张压"))),
      curveSys(new QwtPlotCurve(tr("收缩压"))),
      curveAvg(new QwtPlotCurve(tr("平均值"))),
      curveHR(new QwtPlotCurve(tr("心率")))
{
    // 设置画布背景
    setCanvasBackground(QColor("#121824"));

    // 设置标题
    QwtText plotTitle(tr("趋势图"));
    plotTitle.setColor(Qt::white);  // 设置颜色
    plotTitle.setFont(QFont("Microsoft YaHei", 16, QFont::Bold));
    setTitle(plotTitle);

    // 设置 X 轴标题（白色）
    QwtText xTitle(tr("时间"));
    xTitle.setColor(Qt::white);
    xTitle.setFont(QFont("Microsoft YaHei", 14));
    setAxisTitle(QwtPlot::xBottom, xTitle);

    // 设置 Y 轴标题（白色）
    QwtText yTitle(tr("数值"));
    yTitle.setColor(Qt::white);
    yTitle.setFont(QFont("Microsoft YaHei", 14));
    setAxisTitle(QwtPlot::yLeft, yTitle);

    // 设置刻度字体
    QFont axisFont("Microsoft YaHei", 11);
    setAxisFont(QwtPlot::xBottom, axisFont);
    setAxisFont(QwtPlot::yLeft, axisFont);

    // 设置坐标轴刻度值颜色（必须设置 Palette）
    QPalette whitePalette;
    whitePalette.setColor(QPalette::WindowText, Qt::white);

    axisWidget(QwtPlot::xBottom)->setPalette(whitePalette);
    axisWidget(QwtPlot::yLeft)->setPalette(whitePalette);

    // 设置网格线为亮灰色
    QwtPlotGrid *grid = new QwtPlotGrid;
    grid->setPen(QPen(QColor("#CCCCCC"), 0.5, Qt::DashLine));
    grid->attach(this);

    // 添加图例
    QwtLegend *legend = new QwtLegend();
    legend->setFrameStyle(QFrame::NoFrame);
    legend->setStyleSheet("color: white; font-size: 14px;");
    insertLegend(legend, QwtPlot::RightLegend);

    //时间刻度 —— 只一行“MM-dd HH:mm”
    auto *dateScale = new QwtDateScaleDraw(Qt::LocalTime);
    dateScale->setDateFormat(QwtDate::Minute, "MM-dd HH:mm");
    setAxisScaleDraw(QwtPlot::xBottom, dateScale);
    setAxisScaleEngine(QwtPlot::xBottom, new QwtDateScaleEngine(Qt::LocalTime));
    enableAxis(QwtPlot::xBottom, true);

    // 固定 Y 轴范围
    setAxisScale(QwtPlot::yLeft, 0, 200);

    //  QSS 调整
    this->setStyleSheet(R"(
                        QwtLegend, QwtLegend QLabel {
                        color: white;
                        font-size: 10px;
                        }
                        QwtScaleWidget {
                        color: white;
                        font-size: 10px;
                        }
                        )");

    // 设置曲线样式
    setupCurves();
    retranslate();
}

void TrendChart::setupCurves() {
    // 舒张压曲线
    curveDia->setPen(QPen(QColor(0, 170, 255), 2));
    curveDia->setRenderHint(QwtPlotItem::RenderAntialiased, true);
    curveDia->attach(this);

    // 收缩压曲线
    curveSys->setPen(QPen(QColor(255, 85, 0), 2));
    curveSys->setRenderHint(QwtPlotItem::RenderAntialiased, true);
    curveSys->attach(this);

    // 平均值曲线
    curveAvg->setPen(QPen(QColor(255, 255, 0), 2));
    curveAvg->setRenderHint(QwtPlotItem::RenderAntialiased, true);
    curveAvg->attach(this);

    // 心率曲线
    curveHR->setPen(QPen(QColor(0, 200, 150), 2));
    curveHR->setRenderHint(QwtPlotItem::RenderAntialiased, true);
    curveHR->attach(this);
}

void TrendChart::setData(const QVector<MeasurementData> &data) {
    m_data = data;
    qDebug() << "[TrendChart::setData] 数据条数:" << data.size();

    updatePlot();
}

void TrendChart::updatePlot() {
    if (m_data.isEmpty()) {
        replot();
        return;
    }

    // 准备数据点容器
    QVector<QPointF> diaPoints, sysPoints, avgPoints, hrPoints;

    // 填充数据点
    for (const auto &item : m_data) {
        QDateTime dt = QDateTime::fromString(item.timestamp, "yyyy-MM-dd HH:mm:ss");

        if (!dt.isValid())
            continue;
        double t = dt.toMSecsSinceEpoch();
        double dia = item.sensorDiastolic.toDouble();
        double sys = item.sensorSystolic .toDouble();
        double avg = item.sensorAvg      .toDouble();
        double hr  = item.heartRate      .toDouble();

        diaPoints.append({t, dia});
        sysPoints.append({t, sys});
        avgPoints.append({t, avg});
        hrPoints .append({t, hr});
    }

    // 如果解析后确实没有任何点，就清空曲线并直接重绘
    if (diaPoints.isEmpty()) {
        curveDia->setSamples({});
        curveSys->setSamples({});
        curveAvg->setSamples({});
        curveHR->setSamples({});
        replot();
        return;
    }

    // 设置曲线数据
    curveDia->setSamples(diaPoints);
    curveSys->setSamples(sysPoints);
    curveAvg->setSamples(avgPoints);
    curveHR->setSamples(hrPoints);

    // 设置 X 轴范围（现在肯定有至少一个点）
    double xMin = diaPoints.first().x();
    double xMax = diaPoints.last().x();
    setAxisScale(QwtPlot::xBottom, xMin, xMax);

    replot();
}

void TrendChart::retranslate() {
    // 标题
    QwtText t(tr("趋势图"));
    t.setColor(Qt::white);
    t.setFont(QFont("Microsoft YaHei",16,QFont::Bold));
    setTitle(t);

    // 坐标轴
    QwtText xt(tr("时间"));
    xt.setColor(Qt::white); xt.setFont(QFont("Microsoft YaHei",14));
    setAxisTitle(xBottom, xt);

    QwtText yt(tr("数值"));
    yt.setColor(Qt::white); yt.setFont(QFont("Microsoft YaHei",14));
    setAxisTitle(yLeft, yt);

    // Legend
    curveDia->setTitle(tr("舒张压"));
    curveSys->setTitle(tr("收缩压"));
    curveAvg->setTitle(tr("平均值"));
    curveHR ->setTitle(tr("心率"));

    replot();  // 自动刷新 legend
}

/*******MeasurementTrendWidget 类实现*********/

MeasurementTrendWidget::MeasurementTrendWidget(const MeasurementData &currentResult, QWidget *parent)
    : FramelessWindow(parent), m_currentResult(currentResult) , m_chart(new TrendChart(this)){

    // 初始化界面参数，获取屏幕分辨率
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();

    // 计算缩放比例
    scaleX = static_cast<float>(screenGeometry.width()) / 1024.0f;
    scaleY = static_cast<float>(screenGeometry.height()) / 600.0f;
    qDebug() << "Scale Y: " << scaleY;

    // 设置窗口初始大小
//    this->resize(1024 * scaleX, 600 * scaleY);  // 设置为基于目标分辨率的大小
    this->showFullScreen();

    // 加载图标（需要在 .qrc 中注册资源）
    m_icons[0].load(":/image/IconParkOutlineMaximum.png");
    m_icons[1].load(":/image/IconParkOutlineMin.png");
    m_icons[2].load(":/image/IconParkOutlineAverage.png");
    m_icons[3].load(":/image/FaHeartbeat.png");

    // 初始化渐变
    m_titleGrad.setStart(0, 0);
    m_titleGrad.setFinalStop(0, 100);
    m_titleGrad.setColorAt(0, QColor(0, 120, 255));
    m_titleGrad.setColorAt(1, QColor(0, 60, 200));

    m_btnGrads[0].setStart(0, 0);
    m_btnGrads[0].setFinalStop(0, 60);
    m_btnGrads[0].setColorAt(0, QColor(0, 160, 255));
    m_btnGrads[0].setColorAt(1, QColor(0, 100, 200));

    m_btnGrads[1].setStart(0, 0);
    m_btnGrads[1].setFinalStop(0, 60);
    m_btnGrads[1].setColorAt(0, QColor(0, 200, 150));
    m_btnGrads[1].setColorAt(1, QColor(0, 150, 100));

    // 加载历史数据
    loadHistoryData();

    // 组装界面
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20 * scaleX, 20 * scaleY, 20 * scaleX, 20 * scaleY);
    mainLayout->setSpacing(20 * scaleY);

    mainLayout->addWidget(createTitleWidget());

    // 卡片布局
    QHBoxLayout *cardLayout = new QHBoxLayout();
    cardLayout->setSpacing(20 * scaleX);
    cardLayout->setAlignment(Qt::AlignCenter);
    for (int i = 0; i < 4; ++i) {
        cardLayout->addWidget(createCardWidget(i, m_currentResult));
    }
    mainLayout->addLayout(cardLayout);

    // 趋势图
    mainLayout->addWidget(m_chart);

    // 按钮
    mainLayout->addWidget(createButtonsWidget());

    // 设置窗口属性
    setStyleSheet("background-color: #0B1018;");
    retranslateUi();
}

MeasurementTrendWidget::~MeasurementTrendWidget() {
    // 确保资源正确释放
}

QWidget* MeasurementTrendWidget::createTitleWidget() {

    // 顶部栏
    QWidget *topBar = new QWidget(this);
    topBar->setStyleSheet(R"(
      QWidget {
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

    topBar->setFixedHeight(40* scaleY);

    backButton = new QPushButton(this);
    connect(backButton, &QPushButton::clicked, this, &MeasurementTrendWidget::trendReturnMeasure);
    backButton->setIcon(QIcon(":/image/icons8-return100.png"));
    backButton->setIconSize(QSize(30 * scaleX, 30 * scaleY));
    backButton->setFixedSize(65 * scaleX, 30 * scaleY);
    backButton->setFlat(true);
    backButton->setStyleSheet(R"(
                               QPushButton {
                               border: none;
                               background-color: transparent;
                               border-radius: 20px;
                               }
                               QPushButton:pressed {
                               background-color: rgba(255, 255, 255, 0.2);
                               }
                               )");
//    titleLabel->setAlignment(Qt::AlignVCenter);

    QPushButton *btnSettings = new QPushButton(this);
    btnSettings->setIcon(QIcon(":/image/icons8-shezhi.png"));
    btnSettings->setIconSize(QSize(30 * scaleX, 30 * scaleY));
    btnSettings->setFlat(true); //扁平化
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
    connect(btnSettings, &QPushButton::clicked, this, [=](){
          settingsWidgetResult = new SettingsWidget();
          settingsWidgetResult->setWindowFlags(Qt::Window);
          connect(settingsWidgetResult,&SettingsWidget::requestDelete,this,&MeasurementTrendWidget::closeSettingsWidget);
          settingsWidgetResult->show();
          settingsWidgetResult->raise();
          this->hide();
    });  // 连接点击事件到槽函数

    // 创建标题标签
    QVBoxLayout *tittleLayout = new QVBoxLayout();
    QLabel *labelSensorID = new QLabel(this);
    labelSensorID->setAlignment(Qt::AlignCenter);
    labelSensorID->setText(fetchSensorIds());
    QFont sensorFont("Microsoft YaHei", 20, QFont::Bold);  // 设置字号22
    labelSensorID->setFont(sensorFont);
    labelSensorID->setStyleSheet("color: #E0F7FA; background-color: transparent; border: none;");

    summaryTitleLabel = new QLabel(tr("(测量数据汇总)"), this);
    summaryTitleLabel->setAlignment(Qt::AlignCenter); // 文本居中显示
    QFont titleFont("Microsoft YaHei", 12);  // 设置字号14
    summaryTitleLabel->setFont(titleFont);
    summaryTitleLabel->setStyleSheet("color: #E0F7FA; background-color: transparent; border: none;");

    tittleLayout->addWidget(labelSensorID);
    tittleLayout->addWidget(summaryTitleLabel);

    QHBoxLayout *topLayout = new QHBoxLayout(topBar);
    topLayout->addWidget(backButton);
    topLayout->addStretch();
    topLayout->addLayout(tittleLayout);
    topLayout->addStretch();
    topLayout->addSpacing(35);
    topLayout->addWidget(btnSettings);
    topLayout->setContentsMargins(10 * scaleX, 0, 10 * scaleX, 0);


    return topBar;
}

QWidget* MeasurementTrendWidget::createCardWidget(int dataIndex, const MeasurementData &result) {
    QString titles[] = {tr("舒张压"), tr("收缩压"), tr("平均值"), tr("心率")};
    double values[] = {
        result.sensorDiastolic.toDouble(),
        result.sensorSystolic .toDouble(),
        result.sensorAvg      .toDouble(),
        result.heartRate      .toDouble()
    };

    QString units[] = {tr("mmHg"), tr("mmHg"), tr("mmHg"), tr("次/分")};

    QWidget *cardWidget = new QWidget(this);
    cardWidget->setFixedSize(200 * scaleX, 135 * scaleY);
    cardWidget->setStyleSheet("background-color: #101827; border-radius: 8px; border: 1px solid #334155;");

    QVBoxLayout *layout = new QVBoxLayout(cardWidget);
    layout->setContentsMargins(15 * scaleX, 15 * scaleY, 15 * scaleX, 15 * scaleY);
    layout->setSpacing(15 * scaleY);

    // 图标
    QLabel *iconLabel = new QLabel(this);
    iconLabel->setPixmap(m_icons[dataIndex].scaled(
        25 * scaleX, 25 * scaleY, Qt::KeepAspectRatio, Qt::SmoothTransformation
    ));
    iconLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(iconLabel);

    // 标题
    QLabel *titleLabel = new QLabel(titles[dataIndex], this);
    titleLabel->setStyleSheet("color: white; font-size: 16px; font-weight: bold;");
    titleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(titleLabel);
    m_cardTitleLabels.append(titleLabel);

    QWidget *valueContainer = new QWidget(cardWidget);
    valueContainer->setObjectName("valueContainer");
    valueContainer->setStyleSheet(R"(
        QWidget#valueContainer {
            border: 1px solid #334155;
            border-radius: 4px;
            background-color: transparent;
        }
    )");

    // 数字
    QLabel *lblValue = new QLabel(QString::number(values[dataIndex], 'f', 1), this);
    lblValue->setStyleSheet("color: #4CD964; font-size: 22px; font-weight: bold;border: none;");
    lblValue->setAlignment(Qt::AlignCenter);

    // 单位
    QLabel *lblUnit = new QLabel(units[dataIndex], this);
    lblUnit->setStyleSheet("color: white; font-size: 14px;border: none;");
    lblUnit->setAlignment(Qt::AlignCenter);
    m_cardUnitLabels.append(lblUnit);

    QHBoxLayout *hLayout = new QHBoxLayout(valueContainer);
    hLayout->addStretch();
    hLayout->addWidget(lblValue);
    hLayout->addWidget(lblUnit);
    hLayout->addStretch();
    layout->addWidget(valueContainer);

    return cardWidget;
}

QWidget* MeasurementTrendWidget::createTrendChartWidget() {
    m_trendChart = new TrendChart(this);
    m_trendChart->setData(m_historyData);
    m_trendChart->setMinimumHeight(300 * scaleY);
    return m_trendChart;
}

QWidget* MeasurementTrendWidget::createButtonsWidget() {
    QWidget *btnWidget = new QWidget(this);
    btnWidget->setFixedHeight(35 * scaleY);
    btnWidget->setStyleSheet(R"(
        background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #1a202c, stop:1 #0f172a);
        border-radius: 8px;
        border: 1px solid #334155;
    )");

    QHBoxLayout *layout = new QHBoxLayout(btnWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(60 * scaleX);
    layout->setAlignment(Qt::AlignCenter);

    // 按钮1：返回主界面
    homeButton = new QPushButton(tr("返回主界面"), this);
    homeButton->setFixedSize(180 * scaleX, 30 * scaleY);
    homeButton->setIcon(QIcon(":/image/icons8-home.png"));
    homeButton->setStyleSheet(R"(
        QPushButton {
            background-color: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #4CAF50, stop:1 #2C6E00);
            color: white;
            font-size: 14px;
            font-weight: bold;
            border-radius: 15px;
            border: 1px solid #388E3C;
        }

        QPushButton:pressed {
            background-color: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #388E3C, stop:1 #2C6E00);
        }
    )");
    // 连接 homeButton 点击事件
    QObject::connect(homeButton, &QPushButton::clicked, this, [this]() {
        this->hide();  // 隐藏当前测量界面
        if (globalLoginWindowPointer) {
            globalLoginWindowPointer->show();  // 显示主界面
        }
        QObject::disconnect(this); // 断开所有与当前窗口相关的信号与槽连接
        this->close();  // 关闭当前测量数据界面
        emit closeAllWindow();
        deleteLater();  // 延迟删除当前窗口对象
    });

    //按钮2：导出数据
    exportButton = new QPushButton(tr("导出数据"), this);
    exportButton->setFixedSize(180 * scaleX, 30 * scaleY);
    exportButton->setIcon(QIcon(":/image/icons8-excel-100.png"));
    exportButton->setStyleSheet(R"(
        QPushButton {
            background-color: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #2196F3, stop:1 #1976D2);
            color: white;
            font-size: 14px;
            font-weight: bold;
            border-radius: 15px;
            border: 1px solid #C62828;
        }

        QPushButton:pressed {
            background-color: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #C62828, stop:1 #D32F2F);
        }
    )");
    connect(exportButton, &QPushButton::clicked, this, &MeasurementTrendWidget::exportData);

    // 按钮3：关机
    shutdownButton = new QPushButton(tr("关  机"), this);
    shutdownButton->setFixedSize(180 * scaleX, 30 * scaleY);
    shutdownButton->setIcon(QIcon(":/image/icons8-shutdown.png"));
    shutdownButton->setStyleSheet(R"(
        QPushButton {
            background-color: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #F44336, stop:1 #D32F2F);
            color: white;
            font-size: 14px;
            font-weight: bold;
            border-radius: 15px;
            border: 1px solid #C62828;
        }

        QPushButton:pressed {
            background-color: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #C62828, stop:1 #D32F2F);
        }
    )");
    connect(shutdownButton, &QPushButton::clicked, this, &MeasurementTrendWidget::shutdownApp);



    layout->addWidget(homeButton);
    layout->addStretch();
    layout->addWidget(exportButton);
    layout->addStretch();
    layout->addWidget(shutdownButton);

    return btnWidget;
}

void MeasurementTrendWidget::loadHistoryData()
{
    m_historyData.clear();

    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        qWarning() << "数据库未打开，无法加载历史数据";
        return;
    }

    // 查询最近 7 天、指定 sensorId 的记录
    QSqlQuery query(db);
    query.prepare(R"(
        SELECT systolic,
               diastolic,
               avg_value,
               heart_rate,
               timestamp
          FROM measurements_data_update
         WHERE datetime(timestamp) >= datetime('now','-7 days')
         ORDER BY datetime(timestamp) ASC
    )");

    if (!query.exec()) {
        qWarning() << "加载趋势数据失败：" << query.lastError().text();
        return;
    }

    // 组装成 MeasurementData
    while (query.next()) {
        MeasurementData d;
        d.sensorSystolic  = query.value(0).toString();
        d.sensorDiastolic = query.value(1).toString();
        d.sensorAvg       = query.value(2).toString();
        d.heartRate       = query.value(3).toString();
        d.timestamp       = query.value(4).toString();
        m_historyData.push_back(d);
    }
    qDebug() << "历史数据加载完成，条数：" << m_historyData.size();

    // 把数据交给 TrendChart 去画
    m_chart->setData(m_historyData);
}

void MeasurementTrendWidget::saveCurrentResult() {
    // 实际项目中应保存到数据库
    qDebug() << "保存当前测量结果到数据库";
}

void MeasurementTrendWidget::onStartNewMeasurement() {
    qDebug() << "开始新测量";
//    emit startNewMeasurement();
    close();
}

void MeasurementTrendWidget::closeSettingsWidget(){
    settingsWidgetResult->deleteLater();  // 安全删除
    settingsWidgetResult = nullptr;  
    //重新给它打上 FramelessHint
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    this->showFullScreen();
}

void MeasurementTrendWidget::shutdownApp() {
    // 调用 Qt 的退出应用函数
    QCoreApplication::quit();
}

void MeasurementTrendWidget::exportData()
{
    // 1) 让用户选一个 CSV 文件
    QString path = QFileDialog::getSaveFileName(
        this,
        tr("导出最近7天数据"),
        QDir::homePath(),
        tr("CSV 文件 (*.csv)"));
    if (path.isEmpty()) return;
    if (!path.endsWith(".csv", Qt::CaseInsensitive))
        path += ".csv";

    // 2) 打开输出文件
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("错误"), tr("无法创建文件：%1").arg(path));
        return;
    }
    QTextStream out(&file);

    // 3) 写入 CSV 表头（含传感器号）
    out << tr("传感器号,舒张压,收缩压,平均值,心率,测量时间") << "\n";

    // 4) 从 SQLite 查询——假设字段名 sensor_id, timestamp, diastolic, systolic, average, heartRate
    QSqlQuery query;
    bool ok = query.exec(R"(
        SELECT sensor_id,
               systolic,
               diastolic,
               avg_value,
               heart_rate,
               timestamp
          FROM measurements_data_update
         WHERE datetime(timestamp) >= datetime('now','-7 days')
      ORDER BY datetime(timestamp) ASC
    )");

    if (!ok) {
        QMessageBox::critical(this, tr("查询失败"),
            tr("数据库查询出错：%1").arg(query.lastError().text()));
        file.close();
        return;
    }

    // 5) 遍历结果，写入每一行
    while (query.next()) {
        QString sid = query.value(0).toString();
        QString dia  = query.value(1).toString();
        QString sys  = query.value(2).toString();
        QString avg  = query.value(3).toString();
        QString hr   = query.value(4).toString();
        QString ts  = query.value(5).toString();

        out
          << sid << ","
          << dia << ","
          << sys << ","
          << avg << ","
          << hr  << ","
          << ts << "\n";
    }

    file.close();

    // 6) 提示用户导出成功
    QMessageBox::information(this, tr("导出完成"),
        tr("最近7天的数据（含传感器号）已导出至：\n%1").arg(path));
}

QString MeasurementTrendWidget::fetchSensorIds() const
{
    // 取默认连接
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        qWarning() << "数据库未打开！";
        return tr("传感器ID未知");
    }

    QSqlQuery query(db);
    if (!query.exec("SELECT sensor_id FROM sensor_info")) {
        qWarning() << "查询 sensor_info 失败：" << query.lastError().text();
        return tr("传感器ID未知");
    }

    QStringList sensors;
    while (query.next()) {
        QString id = query.value(0).toString().trimmed();
        if (!id.isEmpty())
            sensors << id;
    }

    if (sensors.isEmpty())
        return tr("传感器ID查询为空");

    return sensors.join(", ");
}

void MeasurementTrendWidget::retranslateUi()
{
    // 顶部文字
    summaryTitleLabel->setText(tr("(测量数据汇总)"));

    // 卡片上的标题和单位
    QStringList titles = { tr("舒张压"), tr("收缩压"), tr("平均值"), tr("心率") };
    QStringList units  = { tr("mmHg"),      tr("mmHg"),      tr("mmHg"),       tr("次/分") };
    for (int i = 0; i < m_cardTitleLabels.size() && i < titles.size(); ++i){
        m_cardTitleLabels[i]->setText(titles[i]);
        m_cardUnitLabels[i]->setText(units[i]);
    }

    // 底部按钮
    homeButton   ->setText(tr("返回主界面"));
    exportButton ->setText(tr("导出数据"));
    shutdownButton->setText(tr("关  机"));

    // 趋势图内部的文字
    m_chart->retranslate();
}

void MeasurementTrendWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        // 1) 先把所有界面文字都翻译一遍
        retranslateUi();
    }

    FramelessWindow::changeEvent(event);
}
