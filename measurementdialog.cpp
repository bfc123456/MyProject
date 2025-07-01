#include "measurementdialog.h"
#include <QRandomGenerator>
#include <QGuiApplication>
#include <QScreen>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

MeasurementDialog::MeasurementDialog(const QString &sensorId,double signalStrength, QWidget *parent)
    : QDialog(parent), m_sensorId(sensorId), initialSignalStrength(signalStrength), progressValue(0)
{
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);

    // 获取屏幕尺寸，计算缩放比例
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int screenWidth = screenGeometry.width();
    int screenHeight = screenGeometry.height();
    float scaleX = static_cast<float>(screenWidth) / 1024;
    float scaleY = static_cast<float>(screenHeight) / 600;

    // 设置窗口属性
//    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setFixedSize(500 * scaleX, 400 * scaleY);
    setStyleSheet(R"(
                  /* 主窗口样式 */
                  QDialog {
                  background-color: #333333 ;
                  border-radius: 12px;
                  border: 1px solid #2C3E50;
                  border-radius: 0px;
                  }
                  QWidget#mainWidget {
                  background-color: #78909C  ;
                  border: 1px solid #2C3E50;
                  }

                  /* 消息标签样式 */
                  QLabel#messageLabel {
                  font-size: 14px;
                  color: #B0BEC5;
                  text-align: center;
                  word-wrap: true;
                  margin: 20px 0;
                  }

                  QProgressBar {
                      /* 背景样式 */
                      background-color: #2C3E50;
                      border: 2px solid #1A232E;
                      border-radius: 8px;
                      height: 12px;
                      margin: 0 20px;

                      /* 文字样式（可选） */
                      color: #B0BEC5;
                      text-align: center;
                  }

                  QProgressBar::chunk {
                      /* 进度条颜色（高对比色） */
                      background-color: #00C853;
                      border-radius: 6px;
                  }

                  /* 按钮通用样式 */
                  QPushButton {
                  background-color: #34495E;
                  color: #ECEFF1;
                  border-radius: 6px;
                  padding: 8px 16px;
                  font-size: 14px;
                  border: none;
                  margin: 0 10px;
                  }
                  QPushButton:hover {
                  background-color: #455A64;
                  }
                  QPushButton:pressed {
                  background-color: #2C3E50;
                  }

                  /* 右按钮特殊样式（类似开始测量按钮功能时的颜色） */
                  QPushButton#rightButton {
                  background-color: #00C853;
                  }
                  QPushButton#rightButton:hover {
                  background-color: #00E676;
                  }
                  QPushButton#rightButton:pressed {
                  background-color: #00B24C;
                  }
                  )");

    // 创建整体的Widget容器
    QWidget *mainWidget = new QWidget(this);
    mainWidget->setObjectName("mainWidget");
    mainWidget->setStyleSheet(R"(
        QWidget {
            background-color: rgba(26, 58, 88, 0.8);
            border-radius: 0px;
            border: 1px solid #2C3E50;
        }
    )");

    QWidget* titleBar = new QWidget(this);
    titleBar->setFixedHeight(45 * scaleY);
    titleBar->setStyleSheet(R"(
        background-color: #2196F3;
        border: none;
    )");

    QHBoxLayout* titleLayout = new QHBoxLayout(titleBar);
    titleLayout->setContentsMargins(0, 0, 0, 0);
    titleLayout->setSpacing(0);

    // 创建标题栏标签
    titleLabel = new QLabel(this);
    titleLabel->setObjectName("titleLabel");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setFixedHeight(38*scaleY);
    titleLabel->setStyleSheet(R"(
            background-color: #2196F3;
            color: #FFFFFF;
            color: white;
            font-size: 18px;
            font-weight: 600;
            padding: 8px 16px;
            border-radius: 0px;
            /* 添加微妙的内阴影增强深度感 */
            qproperty-alignment: AlignCenter;
    )");

    QPushButton* closeButton = new QPushButton("✕", titleBar);
    closeButton->setFixedSize(80 * scaleX, 40 * scaleY);
    closeButton->setStyleSheet(R"(
        QPushButton {
            background-color: transparent;
            color: white;
            font-size: 16px;
            font-weight: bold;
            border: none;
        }
    )");

    connect(closeButton, &QPushButton::clicked, this, [=]() {
        emit exitOverlay();  // 发出让主界面取消模糊和遮罩的信号
        this->close();       // 关闭自己
    });

    // 添加组件到布局
    titleLayout->addStretch();
    titleLayout->addSpacing(60 * scaleX);
    titleLayout->addWidget(titleLabel);
    titleLayout->addStretch();  //让标题居中
    titleLayout->addWidget(closeButton);

    // 创建消息标签
    messageLabel = new QLabel(this);
    messageLabel->setStyleSheet(R"(
        color: #9E9E9E;             /* 深灰色字体（可根据需要调整色值） */
        font-size: 18px;           /* 设置字体大小 */
        font-weight: 500;          /* 设置字体粗细 */
    )");
    messageLabel->setAttribute(Qt::WA_TranslucentBackground, true);
    messageLabel->setAlignment(Qt::AlignCenter);
    messageLabel->setFixedHeight(80*scaleY);

    // 创建进度条
    progressBar = new QProgressBar(this);
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    progressBar->setFixedSize(360*scaleX,30*scaleY);
    progressBar->setStyleSheet(R"(
        QProgressBar {
            border: 2px solid #444;
            border-radius: 10px;
            background-color: #2C3E50;
            text-align: center;
            height: 25px;
            color: blue;
        }
        QProgressBar::chunk {
            background-color: #1ABC9C;
            border-radius: 10px;
            width: 100%;  // 让进度块自适应宽度，保证连续性
        }
    )");

    // 创建按钮
    leftButton = new QPushButton(tr("返回"), this);
    rightButton = new QPushButton(tr("开始测量"), this);
    leftButton->setFixedSize(180*scaleX,35*scaleY);
    rightButton->setFixedSize(180*scaleX,35*scaleY);
    rightButton->setStyleSheet(R"(
        QPushButton{
            /* 偏绿色背景，使用渐变色增强立体感 */
            background: #66BB6A;
            color: white;
            font-size: 20px;
            font-weight: 500;
            border-radius: 6px;
            padding: 8px 16px;
            min-height: 32px;
            border: 1px solid rgba(0,0,0,0.1);
        }

        QPushButton:pressed {
            /* 按下时颜色变暗，添加下沉效果 */
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #3CC954, stop:1 #0CC14D);
        }

        QPushButton:disabled {
            /* 禁用状态样式 */
            background: #A0A0A0;
            color: #E0E0E0;
        }
    )");
    leftButton->setStyleSheet(R"(
        QPushButton {
            /* 偏淡蓝色背景，使用渐变色增强立体感 */
            background: #42A5F5;
            color: white;
            font-size: 20px;
            font-weight: 500;
            border-radius: 6px;
            padding: 8px 16px;
            min-height: 32px;
            border: 1px solid rgba(0,0,0,0.1);
        }

        QPushButton:pressed {
            /* 按下时颜色变暗，添加下沉效果 */
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #3A80D2, stop:1 #2B73C9);return
        }

        QPushButton:disabled {
            /* 禁用状态样式 */
            background: #A0A0A0;
            color: #E0E0E0;
        }
    )");

    // 按钮布局
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    buttonLayout->addWidget(leftButton);
    buttonLayout->addSpacing(50);
    buttonLayout->addWidget(rightButton);
    buttonLayout->addStretch();

    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(mainWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(titleBar);
    mainLayout->addSpacing(45);
    mainLayout->addWidget(messageLabel, 0, Qt::AlignCenter);
    mainLayout->addWidget(progressBar, 0, Qt::AlignCenter);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addSpacing(45);

    // 将mainWidget设置为对话框的主部件
    QVBoxLayout *dialogLayout = new QVBoxLayout(this);
    dialogLayout->addWidget(mainWidget);
    dialogLayout->setContentsMargins(20, 20, 20, 20);
    progressTimer = new QTimer(this);

    currentState = STATE_READY;
    // 首次填充文本
    retranslateUi();

    // 连接信号与槽
    connect(progressTimer, &QTimer::timeout, this, &MeasurementDialog::updateProgress);
    connect(leftButton, &QPushButton::clicked, this, &MeasurementDialog::onCancelButtonClicked);
    connect(rightButton, &QPushButton::clicked, this, &MeasurementDialog::startMeasurement);
}

MeasurementDialog::~MeasurementDialog()
{

}

void MeasurementDialog::retranslateUi()
{
    switch (currentState) {
        case STATE_READY:
            titleLabel->setText(tr("信号已到达测量标准"));
            messageLabel->setText(
                tr("信号强度已符合要求（%1%）\n是否开始测量？")
                  .arg(initialSignalStrength)
            );
            leftButton ->setText(tr("返回"));
            rightButton->setText(tr("开始测量"));
            progressBar->setVisible(false);
            break;

        case STATE_MEASURING:
            titleLabel->setText(tr("测量进行中"));
            messageLabel->setText(tr("量测正在进行中，请保持位置..."));
            leftButton ->setText(tr("重新测量"));
            rightButton->setText(tr("暂停测量"));
            progressBar->setVisible(true);
            break;

        case STATE_PAUSED:
            titleLabel->setText(tr("测量已暂停"));
            messageLabel->setText(tr("测量已暂停，点击继续..."));
            leftButton ->setText(tr("重新测量"));
            rightButton->setText(tr("继续测量"));
            progressBar->setVisible(false);
            break;

        case STATE_COMPLETED:
            titleLabel->setText(tr("测量完毕"));
            messageLabel->setText(tr("测量完毕，请您点击按钮查看详细结果"));
            leftButton ->setText(tr("重新测量"));
            rightButton->setText(tr("查看详细结果"));
            progressBar->setVisible(false);
            break;
    }
}

void MeasurementDialog::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QDialog::changeEvent(event);
}

void MeasurementDialog::startMeasurement()
{
    currentState = STATE_MEASURING;
//    titleLabel->setText(tr("测量进行中"));
//    messageLabel->setText(tr("量测正在进行中，请保持位置..."));
    progressBar->setVisible(true);
    progressBar->setValue(0);

    // 更改按钮文本和功能
    retranslateUi();

    // 断开旧连接，连接新槽函数
    leftButton->disconnect();
    rightButton->disconnect();
//    connect(leftButton, &QPushButton::clicked, this, &MeasurementDialog::onCancelButtonClicked);
    connect(rightButton, &QPushButton::clicked, this, &MeasurementDialog::onPauseButtonClicked);

    // 启动进度更新
    progressValue = 0;
    progressTimer->start(100);  // 每100ms增加百分之一

    emit measurementStarted();
}

void MeasurementDialog::updateProgress()
{
    // 检查当前信号强度，如果信号不符合要求，停止测量
    if (initialSignalStrength < 75.0) {  // 假设正常的信号设定阈值为 75
        progressTimer->stop();
        currentState = STATE_COMPLETED;
        titleLabel->setText(tr("信号强度不足"));
        messageLabel->setText(tr("当前信号强度不足，无法完成测量，请重新调整位置或检查信号源"));

        // 更改按钮文本和功能
        leftButton->setText(tr("重新测量"));
        rightButton->setText(tr("关闭"));

        // 断开旧连接，连接新槽函数
        leftButton->disconnect();
        rightButton->disconnect();
        connect(leftButton, &QPushButton::clicked, this, &MeasurementDialog::onRestartButtonClicked);
        connect(rightButton, &QPushButton::clicked, this, &MeasurementDialog::onCancelButtonClicked);

        emit measurementInterrupted();  // 发出信号，通知外部测量被中断
        return;  // 退出函数，停止测量
    }

    // 模拟每100ms生成一次肺动脉压力数据
    float currentPressure = generatePulmonaryPressureSample();
    pressureSamples.append(currentPressure);  // 存储样本数据

    // 如果信号强度满足要求，继续更新进度
    progressValue += 1;
    progressBar->setValue(progressValue);

    if (progressValue >= 100) {
        progressTimer->stop();
        currentState = STATE_COMPLETED;
        calculateMeasurementResults(); // 数据收集完成，开始计算指标

        retranslateUi();

        // 断开旧连接，连接新槽函数
        leftButton->disconnect();
        rightButton->disconnect();
        connect(leftButton, &QPushButton::clicked, this, &MeasurementDialog::onRestartButtonClicked);
        connect(rightButton, &QPushButton::clicked, this, &MeasurementDialog::onViewResultButtonClicked);

        m_currentResult = calculateMeasurementResults();  //计算并发送计算的结果
        emit measurementCompleted(m_currentResult);
    }
}


// 生成单个肺动脉压力样本的函数
float MeasurementDialog::generatePulmonaryPressureSample()
{
    // 病患肺动脉高压特征：收缩压>30mmHg，舒张压>15mmHg
    static float baseline = 25.0f;  // 基础压力（高于正常基线）
    static float trend = 0.0f;      // 病理趋势变化（缓慢升高）

    // 生成随机波动（-3.0 到 3.0 之间）
    float randomFluctuation = QRandomGenerator::global()->bounded(6.0f) - 3.0f;

    // 生成呼吸影响（-3.0 到 3.0 之间的正弦波动）
    float respiratoryInfluence = sin(QDateTime::currentMSecsSinceEpoch() / 1000.0f * M_PI) * 3.0f;

    // 病理趋势：模拟病情进展（压力缓慢升高）
    trend += QRandomGenerator::global()->bounded(0.08f) + 0.02f;  // 正向趋势为主
    if (trend < 0.0f) trend = 0.0f;  // 避免趋势下降
    if (trend > 5.0f) trend = 5.0f;  // 限制趋势上限

    // 生成病患肺动脉压力样本（包含病理特征）
    float sample = baseline + trend + randomFluctuation + respiratoryInfluence;

    // 病患压力范围调整（收缩压常见30-50mmHg，舒张压15-30mmHg）
    if (sample < 15.0f) sample = 15.0f;  // 下限提高
    if (sample > 60.0f) sample = 60.0f;  // 上限设置为60mmHg（重度高压）

    return sample;
}

// 计算测量结果的函数
MeasurementData MeasurementDialog::calculateMeasurementResults()
{
    MeasurementData result;

    result.sensorId = m_sensorId;

    // timestamp 是 QString，所以要 toString()
    result.timestamp       = QDateTime::currentDateTime()
                                .toString("yyyy-MM-dd HH:mm:ss");

    if (pressureSamples.isEmpty()) {
        // 处理没有数据的情况
        result.sensorSystolic  = QString::number(0.0, 'f', 2);
        result.sensorDiastolic = QString::number(0.0, 'f', 2);
        result.sensorAvg       = QString::number(0.0, 'f', 2);
        result.heartRate       = QString::number(0.0, 'f', 2);

        return result;
    }

    // 最小值 = 舒张压
    double dia = *std::min_element(
                pressureSamples.begin(), pressureSamples.end());
    // 最大值 = 收缩压
    double sys = *std::max_element(
                pressureSamples.begin(), pressureSamples.end());
    // 平均值 (权重公式)
    double avg = (sys + 2.0 * dia) / 3.0;
    // 心率模拟
    double hr = 60.0 + QRandomGenerator::global()->bounded(400) / 10.0;

    // **转换为QString并保留两位小数**
    result.sensorDiastolic  = QString::number(dia, 'f', 2);
    result.sensorSystolic   = QString::number(sys, 'f', 2);
    result.sensorAvg        = QString::number(avg, 'f', 2);
    result.heartRate        = QString::number(hr, 'f', 2);

    return result;
}

void MeasurementDialog::onStartButtonClicked()
{
    startMeasurement();
}

void MeasurementDialog::onCancelButtonClicked()
{
    this->close();  // 关闭对话框
    emit exitOverlay();
}

// 替换 onPauseButtonClicked 中的 static 变量
void MeasurementDialog::onPauseButtonClicked()
{
    if (currentState == STATE_MEASURING) {
        // 从[测量中]→[已暂停]
        progressTimer->stop();
        currentState = STATE_PAUSED;
    }
    else if (currentState == STATE_PAUSED) {
        // 从[已暂停]→[测量中]
        progressTimer->start(100);
        currentState = STATE_MEASURING;
    }
    // 不管是哪种，点完都刷新一次 UI
    retranslateUi();
    emit measurementPaused(currentState == STATE_PAUSED);
}

void MeasurementDialog::onRestartButtonClicked()
{
    // 重置进度
    progressValue = 0;
    progressBar->setValue(0);

    // 如果已经完成，切换回测量状态
    if (currentState == STATE_COMPLETED) {
        currentState = STATE_MEASURING;
        retranslateUi();
        rightButton->disconnect();
        connect(rightButton, &QPushButton::clicked, this, &MeasurementDialog::onPauseButtonClicked);
    }

    // 启动进度更新
    progressTimer->start(100);

    emit measurementRestarted();
}

void MeasurementDialog::onViewResultButtonClicked()
{
    if (!updateResultToDatabase(m_currentResult)) {
        qWarning() << "更新测量结果到数据库失败，界面不打开。";
        return;
    }

    // 创建结果对话框并传递数据
    MeasurementTrendWidget *resultDialog = new MeasurementTrendWidget(m_currentResult,nullptr);
    connect(resultDialog,&MeasurementTrendWidget::trendReturnMeasure,this,[&, resultDialog = resultDialog]() mutable{
        this->show();
        delete resultDialog;
        // 置空指针，避免野指针
        resultDialog = nullptr;
    });
    connect(resultDialog,&MeasurementTrendWidget::closeAllWindow,this,[=](){

        // 断开信号和槽连接，确保不再触发已销毁的窗口
        QObject::disconnect(resultDialog);
//        qDebug() << "Received closeAllWindow signal, closing related windows...";
        emit closeFollowUpForm();
    });

    // 设置窗口标志，使其置顶显示
    resultDialog->setWindowFlags(resultDialog->windowFlags() | Qt::WindowStaysOnTopHint);

    qDebug()<<"测量结果界面已经构建完毕";
    resultDialog->show();
    qDebug()<<"测量结果界面已显示";

    // 关闭当前对话框或保持打开状态，根据需求决定
    this->hide();
}

bool MeasurementDialog::updateResultToDatabase(const MeasurementData &result)
{
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) return false;

    QSqlQuery q(db);
    q.prepare(R"(
        INSERT INTO measurements_data_update
          (sensor_id, systolic, diastolic, avg_value, heart_rate, timestamp)
        VALUES
          (:sid,      :sys,      :dia,       :avg,      :hr,         :ts)
    )");
    q.bindValue(":sid", result.sensorId);
    q.bindValue(":sys", result.sensorSystolic);
    q.bindValue(":dia", result.sensorDiastolic);
    q.bindValue(":avg", result.sensorAvg);
    q.bindValue(":hr",  result.heartRate);
    q.bindValue(":ts",  result.timestamp);

    if (!q.exec()) {
        qWarning() << "插入失败：" << q.lastError().text();
        return false;
    }

    return true;
}
