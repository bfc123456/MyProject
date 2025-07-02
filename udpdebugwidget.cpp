#include "udpdebugwidget.h"
#include <QPropertyAnimation>
#include <QHostAddress>
#include "CustomMessageBox.h"
#include <QBuffer>
#include <cstring>
#include <QSplitter>
#include <QGuiApplication>
#include <QScreen>

udpDebugWidget::udpDebugWidget(QWidget *parent)
    : FramelessWindow(parent), udpSocket(new QUdpSocket(this)) {

    // 获取屏幕分辨率
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int screenWidth = screenGeometry.width();
    int screenHeight = screenGeometry.height();

    // 计算缩放比例
    scaleX = (float)screenWidth / 1024;
    scaleY = (float)screenHeight / 600;

    this->resize(1024*scaleX, 600*scaleY);
    this->setWindowFlag(Qt::WindowStaysOnTopHint);
    timer.start();

    // 主应用样式
    this->setStyleSheet(R"(
        QWidget {
            background-color: qlineargradient(
                x1: 0, y1: 0, x2: 0, y2: 1,
                stop: 0 rgba(35, 45, 65, 240),   /* 顶部略亮蓝灰 */
                stop: 1 rgba(20, 30, 50, 240)    /* 底部更暗 */
            );
            border: 1px solid rgba(255, 255, 255, 0.08);  /* 柔和外边框 */
            border-radius: 12px;
            padding: 8px;
            color: white; /* 设置默认字体为白色 */
        }

        /* 美化输入框 */
        QLineEdit {
            background-color: #2c3e50;
            border: 2px solid #3e5261;
            border-radius: 8px;
            padding: 6px;
            color: white;
            font-size: 14px;
            font-weight: bold;
        }

        /* 焦点时边框高亮 */
        QLineEdit:focus {
            border: 2px solid #66bbee;
            background-color: #243342;
        }

        /* 占位符颜色 */
        QLineEdit::placeholder {
            color: #a0b6cc;
        }

    )");

    // top bar
    auto topBar = new QWidget(this);
    topBar->setAttribute(Qt::WA_TranslucentBackground);
    topBar->setStyleSheet("background: transparent;");
    auto iconLabel = new QLabel(this);
    iconLabel->setStyleSheet(" background: transparent; border: none;");
    QPixmap pix(":/image/icons8-tingzhen.png");
    // 缩放到合适大小，比如 24×24
    pix = pix.scaled(24 * scaleX, 24 * scaleY, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    iconLabel->setPixmap(pix);

    auto titleLabel = new QLabel(tr("医疗设备管理系统"),this);
    titleLabel->setStyleSheet("font-size: 25px; font-weight: bold; background: transparent; border: none;");
    auto btnClose = new QPushButton(this);
    btnClose->setIcon(QIcon(":/image/icons-close.png"));
    btnClose->setIconSize(QSize(30*scaleX, 30*scaleY));
    btnClose->setFlat(true);
    btnClose->setStyleSheet(R"(
        QPushButton {
            border: none;
            background-color: transparent;
            border-radius: 20px; /* 让 hover/pressed 效果是圆的 */
        }
        QPushButton:pressed {
            background-color: rgba(255, 255, 255, 0.2);
        }
    )");
    connect(btnClose, &QPushButton::clicked, this, &udpDebugWidget::onBtnCloseClicked);

    auto topLayout = new QHBoxLayout(topBar);
    topLayout->addWidget(iconLabel);
    topLayout->addWidget(titleLabel);
    topLayout->addStretch();
    topLayout->addWidget(btnClose);
    topLayout->setContentsMargins(10*scaleX, 10*scaleY, 10*scaleX, 10*scaleY);

    // side widget
    QWidget *sideWidget = new QWidget(this);
    sideWidget->setMinimumWidth(200*scaleX);
    auto ipLabel = new QLabel(tr("本地主机地址:"));
    ipLineEdit = new QLineEdit(this);
    ipLineEdit->setPlaceholderText(tr("请输入本地主机地址"));
    ipLineEdit->setFixedHeight(45*scaleY);
    portLineEdit = new QLineEdit(this);
    portLineEdit->setPlaceholderText(tr("请输入本地主机端口"));
    portLineEdit->setFixedHeight(45*scaleY);

    //虚拟键盘
    currentKeyboard = CustomKeyboard::instance(this);

    // 给每个 QLineEdit 注册一次偏移（如果你想要默认偏移都一样，就写同一个 QPoint）
    currentKeyboard->registerEdit(ipLineEdit, QPoint(50*scaleX,0));
    currentKeyboard->registerEdit(portLineEdit, QPoint(50*scaleX,0));

    connectBtn = new QPushButton(tr("开启"));
    connectBtn->setFixedHeight(45*scaleY);

    connectBtn->setStyleSheet(R"(
    QPushButton {
        background-color: qlineargradient(
            x1:0, y1:0, x2:0, y2:1,
            stop:0 rgba(169, 169, 169, 180), /* 灰色 */
            stop:1 rgba(128, 128, 128, 180)  /* 深灰色 */
        );
        border: 1px solid rgba(169, 169, 169, 0.6); /* 半透明边框 */
        border-radius: 6px;
        color: white;
        font-weight: bold;
        font-size: 14px;
        padding: 8px 20px;
    }

    QPushButton:pressed {
        background-color: qlineargradient(
            stop: 0 rgba(105, 105, 105, 200),
            stop: 1 rgba(169, 169, 169, 200)
        );
        padding-left: 2px;
        padding-top: 2px;
    }
    )");


    auto sideLayout = new QVBoxLayout(sideWidget);
    sideLayout->addWidget(ipLabel);
    sideLayout->addWidget(ipLineEdit);
    sideLayout->addSpacing(20*scaleY);
    sideLayout->addWidget(new QLabel(tr("IP 端口:")));
    sideLayout->addWidget(portLineEdit);
    sideLayout->addSpacing(20*scaleY);
    sideLayout->addWidget(connectBtn);
    sideLayout->addStretch();

    // center plot area
    QWidget *centerWidget = new QWidget(this);
    QVBoxLayout *centerLayout = new QVBoxLayout(centerWidget);
    plot1 = new ModernWavePlot(this);
    plot1->setAutoFillBackground(false);
    plot1->setStyleSheet("background: transparent; border: none; color:white;");

    // 设置样式
    plot1->setLineColor(QColor(100, 180, 255));  // 蓝色线
    plot1->setFillColor(QColor(40, 120, 200, 60));  // 半透明蓝色填充（底部区域）
    plot1->setTitle("Pmax");

    plot2 = new ModernWavePlot(this);
    plot2->setAutoFillBackground(false);
    plot2->setStyleSheet("background: transparent; border: none; color:white;");

    // 设置样式
    plot2->setLineColor(QColor(100, 180, 255));  //曲线颜色
    plot2->setFillColor(QColor(40, 120, 200, 30), -1);   //波形底部填充
    plot2->setTitle(tr("F谐"));

    exportBtn = new QPushButton(tr("导出数据"));
    importBtn = new QPushButton(tr("导入数据"));

    exportBtn->setFixedSize(175*scaleX,45*scaleY);
    importBtn->setFixedSize(175*scaleX,45*scaleY);

    exportBtn->setStyleSheet(R"(
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

    importBtn->setStyleSheet(R"(
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

    auto btnLayout = new QHBoxLayout();

    btnLayout->addStretch();
    btnLayout->addWidget(exportBtn);
    btnLayout->addSpacing(350*scaleX);
    btnLayout->addWidget(importBtn);
    btnLayout->addStretch();


    centerLayout->addWidget(plot1);
    centerLayout->addSpacing(35*scaleY);
    centerLayout->addWidget(plot2);
    centerLayout->addSpacing(20*scaleY);
    centerLayout->addLayout(btnLayout);
    centerLayout->addSpacing(20*scaleY);

    // QSplitter for layout
    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);
    splitter->addWidget(sideWidget);
    splitter->addWidget(centerWidget);
    splitter->setStretchFactor(1, 1);
    splitter->setSizes({ 200, 800 });

    // main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(topBar);
    mainLayout->addWidget(splitter);
    mainLayout->setContentsMargins(10*scaleX, 10*scaleY, 10*scaleX, 10*scaleY);

    connectSignals();
}

void udpDebugWidget::connectSignals(){
    connect(exportBtn, &QPushButton::clicked, this, [this]() {
        QString filePath = QFileDialog::getSaveFileName(this, "Save Hex CSV", "", "CSV Files (*.csv)");
        if (!filePath.isEmpty()) {
            exportHexToCSV(rawDataLog, filePath);  // 导出函数
        }
    });
    connect(importBtn, &QPushButton::clicked, this, &udpDebugWidget::onImportButtonClicked);
    connect(connectBtn, &QPushButton::clicked, this, &udpDebugWidget::onConnectBtnClicked);
    connect(udpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(handleSocketError(QAbstractSocket::SocketError)));

}

void udpDebugWidget::onConnectBtnClicked(){
    QString ip = ipLineEdit->text();
    QString port = portLineEdit->text();

    // 检查连接状态
    if (udpSocket->state() == QAbstractSocket::BoundState) {
        // 断开连接
        udpSocket->close();
        connectBtn->setText(tr("连接")); // 修改按钮文本为 "连接"
        qDebug() << "UDP connection closed.";
    }else{

    //验证IP与端口
    if(!isVaildIP(ip)){
       CustomMessageBox dlg(this,tr("错误"),tr("请再次检查输入"), { tr("确定") },350 * scaleX);
       dlg.exec();
       return;
    }

    if(!isVaildPort(port)){
        CustomMessageBox dlg(this,tr("错误"),tr("请再次检查输入"), { tr("确定") },350 * scaleX);
        dlg.exec();
        return;
    }

    //如果验证通过，继续建立连接
    QHostAddress address(ip);
    quint16 portNumber = port.toUInt();

    //绑定到指定端口
    if(!udpSocket->bind(address,portNumber)){
        qDebug()<<"Failed to bind to port!";
        return;
    }

    connectBtn->setText(tr("断开"));
    qDebug()<<"Listening on"<<ip<<":"<<portNumber;

    //连接成功后启动计时器
    timer.start();  //启动计时器

    connect(udpSocket,&QUdpSocket::readyRead,this,&udpDebugWidget::ProcessReceivedData);

  }
}

void udpDebugWidget::ProcessReceivedData(){
    while (udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(int(udpSocket->pendingDatagramSize()));
        udpSocket->readDatagram(datagram.data(), datagram.size());

//        qDebug() << "Received Datagram (Hex):" << datagram.toHex();

        if (datagram.size() == sizeof(float) * 6) {

        // 解析所有 6 个 float（大端）
        QVector<float> receivedFloats;
        for (int i = 0; i < 6; ++i) {
            float value = parseBigEndianFloat(datagram, i * 4);
            receivedFloats.append(value);
//            qDebug() << "Float" << i << ":" << value;
        }

//        float float1 = receivedFloats[0];
//        float float2 = receivedFloats[1];

//        qDebug() << "Float1 in decimal:" << float1;
//        qDebug() << "Float2 in decimal:" << float2;

        // 更新 UI 曲线
        qint64 now = timer.elapsed();
        rawDataLog.append({ now, datagram });

        updatePlot1(receivedFloats[0], now);
        updatePlot2(receivedFloats[1], now);
    } else {
            // 错误数据长度，打印警告并记录十六进制内容
            qWarning() << "Invalid datagram size:" << datagram.size();
            qWarning() << "Raw Data (Hex):" << datagram.toHex();
        }
    }
}


QVector<QPointF> udpDebugWidget::importHexToPlot1(const QString &filePath){
    QVector<QPointF> data;

    QFile file(filePath);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        qWarning()<<"Cannot open file for reading.";
        return data;
    }

    QTextStream in(&file);
    QString header = in.readLine(); //跳过开头

    while (!in.atEnd()){
        QString line = in.readLine();
        QStringList fields = line.split(",");

        //确保每行有两列数据
        if(fields.size()>= 2){
            qint64 timestamp = fields[0].toLongLong();
            QByteArray word1Hex = QByteArray::fromHex(fields[1].trimmed().toUtf8());

            if(word1Hex.size() == 4){
                std::reverse(word1Hex.begin(),word1Hex.end());
                float value;
                memcpy(&value,word1Hex.constData(),4);
                data.append(QPointF(timestamp,value));
            }
        }
    }

    file.close();
    qDebug()<<"Data import from " << filePath;
    return data;
}

QVector<QPointF> udpDebugWidget::importHexToPlot2(const QString &filePath) {
    QVector<QPointF> data;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Cannot open file for reading:" << filePath;
        return data;
    }

    QTextStream in(&file);
    QString header = in.readLine();  // 跳过表头

    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList fields = line.split(',');

        if (fields.size() >= 3) {
            qint64 timestamp = fields[0].toLongLong();
            QByteArray word2Hex = QByteArray::fromHex(fields[2].trimmed().toUtf8());

            if (word2Hex.size() == 4) {
                std::reverse(word2Hex.begin(), word2Hex.end());
                float value;
                memcpy(&value, word2Hex.constData(), 4);
                data.append(QPointF(timestamp, value));
            }
        }
    }

    file.close();
    return data;
}

void udpDebugWidget::exportHexToCSV(const QVector<QPair<qint64, QByteArray>> &dataList, const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Cannot open file for writing:" << filePath;
        return;
    }

    QTextStream out(&file);

    // 表头
    out << "时间(ms),Pmax,F谐,Index序号,Fdac,备用1,备用2\n";

    for (const auto &entry : dataList) {
        qint64 timestamp = entry.first;
        const QByteArray &bytes = entry.second;

        out << timestamp;

        // 分 6 个 word，每个 4 字节，转为十六进制
        for (int i = 0; i < 6; ++i) {
            QByteArray word = bytes.mid(i * 4, 4);
            out << "," << word.toHex();
        }

        out << "\n";
    }

    file.close();
    qDebug() << "Hex data exported to:" << filePath;
}


void udpDebugWidget::onImportButtonClicked() {
    // 弹出文件对话框，选择文件
    QString filePath = QFileDialog::getOpenFileName(this, tr("Open CSV File"), "", tr("CSV Files (*.csv)"));

    if (!filePath.isEmpty()) {
        // 调用已有的 importFromCsv 函数
        QVector<QPointF> plot1Data = importHexToPlot1(filePath);
        QVector<QPointF> plot2Data = importHexToPlot2(filePath);

        //更新两个plot
        if(!curve1){
            curve1 = new QwtPlotCurve("Plot1");
            curve1->attach(plot1);
            curve1->setPen(QPen(QColor(100, 180, 255), 2));  // 蓝线
            curve1->setBrush(QBrush(QColor(40, 120, 200, 60)));  // 底部填充
            curve1->setStyle(QwtPlotCurve::Lines);
            curve1->setRenderHint(QwtPlotItem::RenderAntialiased);
        }
        curve1->setSamples(plot1Data);
        plot1->replot();

        if(!curve2){
            curve2 = new QwtPlotCurve("Plot2");
            curve2->setPen(QPen(QColor(100, 180, 255), 2));  // 蓝线
            curve2->setBrush(QBrush(QColor(40, 120, 200, 60)));  // 底部填充
            curve2->setStyle(QwtPlotCurve::Lines);
            curve2->setRenderHint(QwtPlotItem::RenderAntialiased);
            curve2->attach(plot2);
        }
        curve2->setSamples(plot2Data);

        plot2->replot();
    }
}

void udpDebugWidget::onBtnCloseClicked()
{
    this->close();
}

bool udpDebugWidget::isVaildIP(const QString &ip){
    QHostAddress address(ip);
    return address != QHostAddress::Null;   //检查是否有效的IP地址
}

bool udpDebugWidget::isVaildPort(const QString &port){
    bool ok;
    int portNumber = port.toInt(&ok);
    return ok&& portNumber > 0 && portNumber <=65535;   //检查端口号是否合法
}

//更新plot1波形显示
void udpDebugWidget::updatePlot1(float value, qint64 timestamp) {
    plotData1.append(QPointF(timestamp, value));

    const int maxPoints = 500;
    if (plotData1.size() > maxPoints)
        plotData1.remove(0, plotData1.size() - maxPoints);

    if (!curve1) {
        curve1 = new QwtPlotCurve("Curve1");
        curve1->attach(plot1);
        curve1->setPen(QPen(QColor(100, 180, 255), 2));  // 蓝线
        curve1->setBrush(QBrush(QColor(40, 120, 200, 60)));  // 底部填充
        curve1->setStyle(QwtPlotCurve::Lines);
        curve1->setRenderHint(QwtPlotItem::RenderAntialiased);
        curve1->attach(plot1);
    }

    curve1->setSamples(plotData1);

    plot1->setAxisScale(QwtPlot::xBottom, timestamp - maxPoints, timestamp);
    plot1->replot();
}

void udpDebugWidget::updatePlot2(float value, qint64 timestamp) {
    plotData2.append(QPointF(timestamp, value));

    const int maxPoints = 500;
    if (plotData2.size() > maxPoints)
        plotData2.remove(0, plotData2.size() - maxPoints);

    if (!curve2) {
        curve2 = new QwtPlotCurve("Curve2");
        curve2->setPen(QPen(QColor(100, 180, 255), 2));  // 蓝线
        curve2->setBrush(QBrush(QColor(40, 120, 200, 60)));  // 底部填充
        curve2->setStyle(QwtPlotCurve::Lines);
        curve2->setRenderHint(QwtPlotItem::RenderAntialiased);
        curve2->attach(plot2);
        curve2->attach(plot2);
    }

    curve2->setSamples(plotData2);

    plot2->setAxisScale(QwtPlot::xBottom, timestamp - maxPoints, timestamp);
    plot2->replot();
}


void udpDebugWidget::handleSocketError(QAbstractSocket::SocketError socketError)
{
    qDebug() << "Socket Error: " << socketError;
}

float udpDebugWidget::parseBigEndianFloat(const QByteArray& data, int offset) {
    QByteArray slice = data.mid(offset, 4);
    std::reverse(slice.begin(), slice.end());

    float result;
    memcpy(&result, slice.constData(), 4);
    return result;
}

