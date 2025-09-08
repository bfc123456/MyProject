#include "MeasurementDialog.h"
#include "MeasurementDataProcessor.h"
#include "DeviceAcquisitionWorker.h"
#include <QRandomGenerator>
#include <QGuiApplication>
#include <QScreen>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

// 构造函数：初始化界面、定时器、信号连接
MeasurementDialog::MeasurementDialog(const QString &sensorId, double signalStrength, QWidget *parent)
    : QDialog(parent), m_sensorId(sensorId), initialSignalStrength(signalStrength)
{
    // 1. 窗口基础配置（无边框、固定大小）
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_DeleteOnClose); // 关闭时自动释放内存，避免内存泄漏

    // 2. 屏幕适配（按1024*600基准缩放，适配不同分辨率）
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeo = screen->geometry();
    float scaleX = static_cast<float>(screenGeo.width()) / 1024;
    float scaleY = static_cast<float>(screenGeo.height()) / 600;
    setFixedSize(500 * scaleX, 400 * scaleY);

    // 3. 全局样式表（控制窗口、组件基础风格）
    setStyleSheet(R"(
        QDialog { background-color: #333333; border: 1px solid #2C3E50; }
        QWidget#mainWidget { background-color: rgba(26, 58, 88, 0.8); border: 1px solid #2C3E50; }
    )");

    // 4. 创建标题栏（含标题文本和关闭按钮）
    QWidget *titleBar = new QWidget(this);
    titleBar->setFixedHeight(45 * scaleY);
    titleBar->setStyleSheet("background-color: #2196F3;"); // 标题栏蓝色背景

    QHBoxLayout *titleLayout = new QHBoxLayout(titleBar);
    titleLayout->setContentsMargins(0, 0, 0, 0); // 消除默认边距

    // 标题文本
    titleLabel = new QLabel(this);
    titleLabel->setStyleSheet("color: white; font-size: 18px; font-weight: 600;");
    titleLabel->setAlignment(Qt::AlignCenter); // 文本居中

    // 关闭按钮（"✕"）
    closeButton = new QPushButton("✕", titleBar);
    closeButton->setFixedSize(80 * scaleX, 40 * scaleY);
    closeButton->setStyleSheet("background-color: transparent; color: white; font-size: 16px;");
    connect(closeButton, &QPushButton::clicked, this, &MeasurementDialog::onCancelButtonClicked);

    // 标题栏布局（标题居中，关闭按钮居右）
    titleLayout->addStretch();
    titleLayout->addSpacing(60 * scaleX);
    titleLayout->addWidget(titleLabel);
    titleLayout->addStretch();
    titleLayout->addWidget(closeButton);

    // 5. 创建提示信息标签（显示当前状态提示）
    messageLabel = new QLabel(this);
    messageLabel->setStyleSheet(R"(
        background-color: transparent; /* 背景完全透明，继承父容器背景 */
        color: #ECEFF1;                /* 浅灰白色，深色背景下更清晰 */
        font-size: 18px;
        font-weight: 500;
        text-align: center;           /* 文字居中（可选，确保排版规整） */
    )");
    messageLabel->setAlignment(Qt::AlignCenter); // 文本居中
    messageLabel->setFixedHeight(80 * scaleY);   // 固定高度，避免界面跳动

    // 6. 创建进度条（测量进度可视化）
    progressBar = new QProgressBar(this);
    progressBar->setRange(0, MD_PROGRESS_MAX_VALUE); // 范围0~100
    progressBar->setFixedSize(360 * scaleX, 30 * scaleY); // 固定大小
    progressBar->setStyleSheet(R"(
        QProgressBar {
            border: 2px solid #444; border-radius: 10px;
            background-color: #2C3E50; text-align: center; height: 25px;
        }
        QProgressBar::chunk {
            background-color: #1ABC9C; border-radius: 10px;
        }
    )");
    progressBar->setVisible(false); // 初始隐藏（准备状态不显示）

    // 7. 创建操作按钮（左侧"返回/重新测量"，右侧"开始测量/查看结果"）
    leftButton = new QPushButton(tr("返回"), this);
    rightButton = new QPushButton(tr("开始测量"), this);
    leftButton->setFixedSize(180 * scaleX, 35 * scaleY);
    rightButton->setFixedSize(180 * scaleX, 35 * scaleY);

    // 左侧按钮样式（蓝色：返回/重新测量）
    leftButton->setStyleSheet(R"(
        QPushButton {
            background: #42A5F5; color: white; font-size: 20px;
            border-radius: 6px; border: 1px solid rgba(0,0,0,0.1);
        }
        QPushButton:pressed { background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #3A80D2,stop:1 #2B73C9); }
        QPushButton:disabled { background: #A0A0A0; color: #E0E0E0; }
    )");

    // 右侧按钮样式（绿色：开始测量/查看结果）
    rightButton->setStyleSheet(R"(
        QPushButton {
            background: #66BB6A; color: white; font-size: 20px;
            border-radius: 6px; border: 1px solid rgba(0,0,0,0.1);
        }
        QPushButton:pressed { background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #3CC954,stop:1 #0CC14D); }
        QPushButton:disabled { background: #A0A0A0; color: #E0E0E0; }
    )");

    // 按钮布局（水平居中，两侧留白）
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    buttonLayout->addWidget(leftButton);
    buttonLayout->addSpacing(50); // 按钮间距
    buttonLayout->addWidget(rightButton);
    buttonLayout->addStretch();

    // 8. 主布局组装（将所有组件整合到主容器）
    QWidget *mainWidget = new QWidget(this);
    mainWidget->setObjectName("mainWidget"); // 关联样式表

    QVBoxLayout *mainLayout = new QVBoxLayout(mainWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(titleBar);        // 标题栏
    mainLayout->addSpacing(45);             // 上方留白
    mainLayout->addWidget(messageLabel, 0, Qt::AlignCenter); // 提示信息
    mainLayout->addWidget(progressBar, 0, Qt::AlignCenter);  // 进度条
    mainLayout->addLayout(buttonLayout);    // 操作按钮
    mainLayout->addSpacing(45);             // 下方留白

    // 对话框主布局（给主容器加边距）
    QVBoxLayout *dialogLayout = new QVBoxLayout(this);
    dialogLayout->addWidget(mainWidget);
    dialogLayout->setContentsMargins(20, 20, 20, 20);

    // 10. 按钮信号连接（左侧/右侧按钮点击事件）
    connect(leftButton, &QPushButton::clicked, this, &MeasurementDialog::onLeftButtonClicked);
    connect(rightButton, &QPushButton::clicked, this, &MeasurementDialog::onRightButtonClicked);

    // 11. 初始界面文本（根据准备状态设置文案）
    retranslateUi();
}

// 析构函数：断开信号连接，避免野指针
MeasurementDialog::~MeasurementDialog()
{
    qDebug() << "[MeasurementDialog] 析构：释放资源";
    // 断开与全局线程的所有信号连接（防止线程触发已销毁界面的槽函数）
    if (g_MeasurementDataProcessor) {
        QObject::disconnect(g_MeasurementDataProcessor, nullptr, this, nullptr);
    }
    if (g_DeviceAcquisitionWorker) {
        QObject::disconnect(this, nullptr, g_DeviceAcquisitionWorker, nullptr);
    }
}

// 开始测量：切换状态、启动定时器、初始化线程
void MeasurementDialog::onStartButtonClicked()
{
    qDebug() << "[MeasurementDialog] 开始测量：进入测量状态";
    currentState = STATE_MEASURING;

    // 禁用无关按钮（防止误操作）
    leftButton->setEnabled(false);
    closeButton->setEnabled(false);

    initWorkersAndConnections();
    // 更新界面文案（显示"测量进行中"相关文本）
    // 创建定时器，每100毫秒触发一次
    progress = 0;
    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MeasurementDialog::updateProgress);
    timer->start(MEASUREMENT_TOTAL_DURATION_MS / 100);  // 设置定时器更新频率
    retranslateUi();
}

// 线程与信号初始化：仅首次启动时执行，建立线程间数据流转
void MeasurementDialog::initWorkersAndConnections()
{
    // 前置检查：全局工作对象必须有效
    if (!g_MeasurementDataProcessor || !g_DeviceAcquisitionWorker) {
        MedicalLogger::instance()->writeLog(
            "Threading",
            MedicalLogger::LOG_ERROR,
            "Global worker object not initialized, thread startup aborted",
            " ",
            "System"
        );
        return;
    }

    // 1.2 接收线程→数据处理器（核心数据链路，唯一连接）
    static bool isMag5DataConnected = false;
    if (!isMag5DataConnected) {
        bool ok2 = connect(
                    g_DeviceAcquisitionWorker,
                    &DeviceAcquisitionWorker::mag5DataReceived,
                    g_MeasurementDataProcessor,
                    &MeasurementDataProcessor::parseData,
                    Qt::QueuedConnection // 跨线程
                );
        isMag5DataConnected = ok2;  // 仅当连接成功才标记
        if (!ok2) {
            return;
        }
    }

    // 阶段2：启动数据处理器（使用startMeasurement中初始化的时间基准）
    bool invokeOk = QMetaObject::invokeMethod(
        g_MeasurementDataProcessor, "setMeasuring",
        Qt::QueuedConnection,
        Q_ARG(bool, true)  // 复用前端已初始化的时间
    );
    if (!invokeOk) {
        return;
    }

    // 阶段3：启动UDP接收（先停止旧任务，避免资源冲突）
    // 3.1 停止可能存在的旧接收任务
    QMetaObject::invokeMethod(
        g_DeviceAcquisitionWorker, "stopAcquisition",  // 假设存在停止方法
        Qt::QueuedConnection
    );

    // 3.2 启动新的接收任务
    bool startOk = QMetaObject::invokeMethod(
        g_DeviceAcquisitionWorker, "startAcquisition",
        Qt::QueuedConnection
    );
    if (!startOk) {
        // 回滚状态
        QMetaObject::invokeMethod(g_MeasurementDataProcessor, "setMeasuring", Qt::QueuedConnection, Q_ARG(bool, false));
        return;
    }
    connect(g_MeasurementDataProcessor, &MeasurementDataProcessor::measureFinished,
            this, &MeasurementDialog::onMeasureCompleted,
            Qt::QueuedConnection);
}

// 进度条刷新：根据测量时长计算进度（0~100%）
void MeasurementDialog::updateProgress()
{
    // 非测量状态：停止刷新
    if (currentState != STATE_MEASURING) {
        return;
    }

    // 每次定时器触发时，增加进度
       progress += 80;  // 每次增加 1%，也就是100毫秒更新一次

       // 确保进度不超过100%
       progress = qMin(progress, 100);

       // 更新进度条显示
       progressBar->setValue(progress);
}

// 测量正常结束：总时长到达，通知线程停止
void MeasurementDialog::onMeasureCompleted(const MeasurementData& finalResult)
{
    qDebug() << "[MeasurementDialog] 测量结束：已达30秒总时长";
    // 线程安全：通知数据处理线程停止测量
    QMetaObject::invokeMethod(g_MeasurementDataProcessor, "setMeasuring",
                              Qt::QueuedConnection, Q_ARG(bool, false));
    // 线程安全：通知设备采集线程停止采集
    QMetaObject::invokeMethod(g_DeviceAcquisitionWorker, "stopAcquisition",
                              Qt::QueuedConnection);
    m_currentResult = finalResult;
    onProcessingFinished();
}

// 线程停止后清理：更新界面状态为"测量完成"
void MeasurementDialog::onProcessingFinished()
{
    qDebug() << "[MeasurementDialog] 线程停止：清理界面状态";

    // 更新界面状态
    m_isProcessing = false;
    currentState = STATE_COMPLETED;               // 切换为"测量完成"状态

    // 启用按钮（允许重新测量/查看结果）
    leftButton->setEnabled(true);
    closeButton->setEnabled(true);

    // 更新界面文案（显示"测量完毕"相关文本）
    retranslateUi();
}

// 线程启动后更新：标记测量中状态
void MeasurementDialog::onProcessingStarted()
{
    qDebug() << "[MeasurementDialog] 线程启动：标记测量中";
    m_isProcessing = true;
    leftButton->setEnabled(false);  // 禁用左侧按钮
    closeButton->setEnabled(false); // 禁用关闭按钮
}

// 接收实时测量数据：更新最终结果（数据处理线程触发）
//void MeasurementDialog::appendData(const MeasurementData &data)
//{
//    // 存储最新测量结果（覆盖旧值，最终保留最后一次有效结果）
//    m_currentResult = data;
//}

// 重新测量：重置状态，准备再次测量
void MeasurementDialog::onRestartButtonClicked()
{
    qDebug() << "[MeasurementDialog] 重新测量：重置状态";
    // 停止当前定时器
//    progressTimer->stop();
//    measureTimer->stop();

    // 重置界面状态
    progressBar->setValue(0);       // 进度条归零
    currentState = STATE_READY;     // 切换为"准备"状态
    retranslateUi();                // 更新界面文案

    // 线程安全：通知线程停止当前测量（若仍在运行）
    if (m_isProcessing) {
        QMetaObject::invokeMethod(g_MeasurementDataProcessor, "setMeasuring",
                                  Qt::QueuedConnection, Q_ARG(bool, false));
        QMetaObject::invokeMethod(g_DeviceAcquisitionWorker, "stopAcquisition",
                                  Qt::QueuedConnection);
        m_isProcessing = false;
    }
}

// 查看测量结果：保存结果到数据库，打开结果界面
void MeasurementDialog::onViewResultButtonClicked()
{
    qDebug() << "[MeasurementDialog] 查看结果：保存数据并打开界面";
    // 先保存结果到数据库，失败则不打开界面
    if (!updateResultToDatabase(m_currentResult)) {
        qWarning() << "[MeasurementDialog] 错误：保存结果到数据库失败";
        return;
    }

    // 创建结果界面，传递测量结果
    MeasurementTrendWidget *resultDialog = new MeasurementTrendWidget(m_currentResult, nullptr);
    // 结果界面→当前界面：返回当前界面时，显示当前界面并释放结果界面
    connect(resultDialog, &MeasurementTrendWidget::trendReturnMeasure,
            this, [this, resultDialog]() mutable {
        this->show();
        resultDialog->deleteLater(); // 释放内存
        resultDialog = nullptr;      // 置空指针，避免野指针
    });
    // 结果界面→全局：关闭所有界面时，触发关闭信号强度界面
    connect(resultDialog, &MeasurementTrendWidget::closeAllWindow,
            this, &MeasurementDialog::closePatientSignalStrengthWidget);

    // 结果界面置顶显示
    resultDialog->setWindowFlags(resultDialog->windowFlags() | Qt::WindowStaysOnTopHint);
    resultDialog->show(); // 显示结果界面
    this->hide();         // 隐藏当前测量界面
}

// 右侧按钮点击：根据当前状态触发不同逻辑（开始测量/查看结果）
void MeasurementDialog::onRightButtonClicked()
{
    switch (currentState) {
        case STATE_READY:
            onStartButtonClicked();   // 准备状态：开始测量
            break;
        case STATE_MEASURING:
            // 测量中：不处理（暂不支持暂停）
            break;
        case STATE_COMPLETED:
            onViewResultButtonClicked(); // 完成状态：查看结果
            break;
    }
}

// 左侧按钮点击：根据当前状态触发不同逻辑（返回/重新测量）
void MeasurementDialog::onLeftButtonClicked()
{
    switch (currentState) {
        case STATE_READY:
            onCancelButtonClicked(); // 准备状态：返回上一级
            break;
        case STATE_MEASURING:
        case STATE_COMPLETED:
            onRestartButtonClicked(); // 测量中/完成：重新测量
            break;
    }
}

// 关闭按钮点击：返回上一级界面
void MeasurementDialog::onCancelButtonClicked()
{
    qDebug() << "[MeasurementDialog] 关闭：返回上一级";
    emit exitOverlay(); // 触发返回信号
}

// 界面文本更新：根据当前状态切换标题、提示信息、按钮文本
void MeasurementDialog::retranslateUi()
{
    switch (currentState) {
        case STATE_READY:
            titleLabel->setText(tr("信号已到达测量标准"));
            messageLabel->setText(tr("信号强度已符合要求（%1%）\n是否开始测量？").arg(initialSignalStrength));
            leftButton->setText(tr("返回"));
            rightButton->setText(tr("开始测量"));
            progressBar->setVisible(false); // 准备状态隐藏进度条
            break;

        case STATE_MEASURING:
            titleLabel->setText(tr("测量进行中"));
            messageLabel->setText(tr("测量正在进行中，请保持位置..."));
            leftButton->setText(tr("重新测量"));
            rightButton->setText(tr("测量中..."));
            progressBar->setVisible(true); // 测量中显示进度条
            break;

        case STATE_COMPLETED:
            titleLabel->setText(tr("测量完毕"));
            messageLabel->setText(tr("测量完毕，请您点击按钮查看详细结果"));
            leftButton->setText(tr("重新测量"));
            rightButton->setText(tr("查看详细结果"));
            progressBar->setVisible(false); // 完成状态隐藏进度条
            break;
    }
}

// 语言切换事件：更新界面文本（支持多语言）
void MeasurementDialog::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi(); // 切换语言时更新文案
    }
    QDialog::changeEvent(event);
}

// 测量结果存入数据库：仅测量完成后调用
bool MeasurementDialog::updateResultToDatabase(const MeasurementData &m_currentResult)
{
    // 检查数据库连接状态
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        qWarning() << "[MeasurementDialog] 错误：数据库未打开";
        return false;
    }

    // 准备SQL插入语句（插入传感器ID、收缩压、舒张压等字段）
    QSqlQuery query(db);
    query.prepare(R"(
        INSERT INTO measurements_data_update
          (sensor_id, systolic, diastolic, avg_value, heart_rate, timestamp)
        VALUES
          (:sid,      :sys,      :dia,       :avg,      :hr,         :ts)
    )");

    // 绑定参数（避免SQL注入，确保数据安全）
    query.bindValue(":sid", m_sensorId);          // 传感器ID
    query.bindValue(":sys", m_currentResult.sensorSystolic); // 收缩压
    query.bindValue(":dia", m_currentResult.sensorDiastolic); // 舒张压
    query.bindValue(":avg", m_currentResult.sensorAvg);     // 平均压
    query.bindValue(":hr", m_currentResult.heartRate);      // 心率
    qint64 currentTimestamp = QDateTime::currentSecsSinceEpoch();  // 获取当前时间戳（单位：秒）
    query.bindValue(":ts", currentTimestamp);      // 测量时间戳

    // 执行SQL，返回执行结果
    if (!query.exec()) {
        qWarning() << "[MeasurementDialog] 数据库插入失败：" << query.lastError().text();
        return false;
    }
    return true;
}
