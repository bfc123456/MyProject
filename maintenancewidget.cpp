#include "maintenancewidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSerialPortInfo>
#include <QDebug>
#include <QLabel>
#include <QDateTime>

MaintenanceWidget::MaintenanceWidget(QWidget *parent)
    : QWidget(parent), serialManager(new SerialManager(this))
{
    this->setFixedSize(1024, 600);

    this->setStyleSheet(R"(
        QWidget {
             background-color: qlineargradient(
                 x1: 0, y1: 0, x2: 0, y2: 1,
                 stop: 0 rgba(30, 50, 80, 0.9),     /* 顶部：偏亮蓝灰，透明度 0.9 */
                 stop: 1 rgba(10, 25, 50, 0.75)     /* 底部：深蓝，透明度 0.75 */
             );
            color: white;
            font-size: 14px;
            border-radius: 10px;
        }
    )");

    //设置布局
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(10,30,10,30);

    //左侧串口连接布局
    QWidget *serialconfigwidget = new QWidget();
    serialconfigwidget->setFixedWidth(200);
    QVBoxLayout *serialconfiglayout = new QVBoxLayout(serialconfigwidget);
    serialconfiglayout->setContentsMargins(10,30,10,30);

    // 端口号设置
    QLabel *portLabel = new QLabel(tr("端口号:"));
    portLabel->setStyleSheet("background-color: transparent;");
    portComboBox = new QComboBox(this);
    portComboBox->addItem("COM1");
    portComboBox->addItem("COM2");
    portComboBox->addItem("COM3");
    portComboBox->setFixedSize(80,30);
    QHBoxLayout *portLayout = new QHBoxLayout;
    portLayout->addWidget(portLabel);
    portLayout->addWidget(portComboBox);

    // 波特率设置
    QLabel *baudRateLabel = new QLabel(tr("波特率:"));
    baudRateLabel->setStyleSheet("background-color: transparent;");
    baudRateComboBox = new QComboBox(this);
    baudRateComboBox->addItem("9600");
    baudRateComboBox->addItem("19200");
    baudRateComboBox->addItem("115200");
    baudRateComboBox->setFixedSize(80,30);
    QHBoxLayout *baudRateLayout = new QHBoxLayout;
    baudRateLayout->addWidget(baudRateLabel);
    baudRateLayout->addWidget(baudRateComboBox);

    // 数据位设置
    QLabel *dataBitsLabel = new QLabel(tr("数据位:"));
    dataBitsLabel->setStyleSheet("background-color: transparent;");
    dataBitsComboBox = new QComboBox(this);
    dataBitsComboBox->addItem("5");
    dataBitsComboBox->addItem("6");
    dataBitsComboBox->addItem("7");
    dataBitsComboBox->addItem("8");
    dataBitsComboBox->setFixedSize(80,30);
    QHBoxLayout *dataBitsLayout = new QHBoxLayout;
    dataBitsLayout->addWidget(dataBitsLabel);
    dataBitsLayout->addWidget(dataBitsComboBox);

    // 校验位设置
    QLabel *parityLabel = new QLabel(tr("校验位:"));
    parityLabel->setStyleSheet("background-color: transparent;");
    parityComboBox = new QComboBox(this);
    parityComboBox->addItem("None");
    parityComboBox->addItem("Odd");
    parityComboBox->addItem("Even");
    parityComboBox->setFixedSize(80,30);
    QHBoxLayout *parityLayout = new QHBoxLayout;
    parityLayout->addWidget(parityLabel);
    parityLayout->addWidget(parityComboBox);

    // 停止位设置
    QLabel *stopBitsLabel = new QLabel(tr("停止位:"));
    stopBitsLabel->setStyleSheet("background-color: transparent;");
    stopBitsComboBox = new QComboBox(this);
    stopBitsComboBox->addItem("1");
    stopBitsComboBox->addItem("1.5");
    stopBitsComboBox->addItem("2");
    stopBitsComboBox->setFixedSize(80,30);
    QHBoxLayout *stopBitsLayout = new QHBoxLayout;
    stopBitsLayout->addWidget(stopBitsLabel);
    stopBitsLayout->addWidget(stopBitsComboBox);

    // 流控设置
    QLabel *flowControlLabel = new QLabel(tr("流控制:"));
    flowControlLabel->setStyleSheet("background-color: transparent;");
    flowControlComboBox = new QComboBox(this);
    flowControlComboBox->addItem("None");
    flowControlComboBox->addItem("RTS/CTS");
    flowControlComboBox->addItem("XON/XOFF");
    flowControlComboBox->setFixedSize(80,30);
    QHBoxLayout *flowControlLayout = new QHBoxLayout;
    flowControlLayout->addWidget(flowControlLabel);
    flowControlLayout->addWidget(flowControlComboBox);

    serialconfiglayout->addLayout(portLayout);
    serialconfiglayout->addLayout(baudRateLayout);
    serialconfiglayout->addLayout(dataBitsLayout);
    serialconfiglayout->addLayout(parityLayout);
    serialconfiglayout->addLayout(stopBitsLayout);
    serialconfiglayout->addLayout(flowControlLayout);
    QString comboBoxStyle = R"(
    QComboBox {
        font-family: 'Microsoft YaHei';
        font-size: 12px;
        font-weight: bold;
        color: white;                               /* 当前显示项字体颜色 */
        background-color: rgba(30, 40, 60, 230);     /* 背景色 */
        border: 1px solid rgba(255, 255, 255, 0.2);
        border-radius: 8px;
        padding: 2px 5px;
    }

    QComboBox QAbstractItemView {
        background-color: rgba(30, 40, 60, 240);     /* 下拉菜单背景色 */
        color: white;                                /* 下拉菜单字体颜色 */
        selection-background-color: rgba(100, 160, 230, 180); /* 选中项背景色 */
        selection-color: white;                      /* 选中项字体颜色 */
        border: 1px solid rgba(255, 255, 255, 0.1);
        outline: 0;
    }
    )";

    portComboBox->setStyleSheet(comboBoxStyle);
    baudRateComboBox->setStyleSheet(comboBoxStyle);
    dataBitsComboBox->setStyleSheet(comboBoxStyle);
    parityComboBox->setStyleSheet(comboBoxStyle);
    stopBitsComboBox->setStyleSheet(comboBoxStyle);
    flowControlComboBox->setStyleSheet(comboBoxStyle);

    //连接按钮
    connectButton = new QPushButton(tr("连接串口"));
    connectButton->setIcon(QIcon(":/image/icons8-connect.png"));
    connectButton->setFixedWidth(150);

    serialconfiglayout->addStretch();
    serialconfiglayout->addWidget(connectButton, 0, Qt::AlignHCenter);
    serialconfiglayout->setAlignment(Qt::AlignHCenter);


    //右侧信息显示布局
    QWidget *showdatawidget = new QWidget();
    showdatawidget->setFixedWidth(520);
    QVBoxLayout *showdatalayout = new QVBoxLayout(showdatawidget);
    showdatalayout->setContentsMargins(30,30,30,30);

    QLabel *recivedatalable = new QLabel(tr("接收数据"));
    recivedatalable->setStyleSheet("background-color: transparent;");
    receiveTextEdit = new QTextEdit(this);
    receiveTextEdit->setReadOnly(true);

    QLabel *senddatalabel = new QLabel(tr("发送数据"));
    senddatalabel->setStyleSheet("background-color: transparent;");
    sendTextEdit = new QTextEdit(this);


    showdatalayout->addWidget(recivedatalable);
    showdatalayout->addWidget(receiveTextEdit);
    showdatalayout->addSpacing(10);
    showdatalayout->addWidget(senddatalabel);
    showdatalayout->addWidget(sendTextEdit);

    QHBoxLayout *buttonlayout = new QHBoxLayout();
    buttonlayout->setAlignment(Qt::AlignHCenter);
    sendButton = new QPushButton(tr("发送数据"));
    clearReceiveButton = new QPushButton(tr("清空接收区"));
    clearSendButton = new QPushButton(tr("清空发送区"));
    sendButton->setFixedWidth(135);
    clearReceiveButton->setFixedWidth(135);
    clearSendButton->setFixedWidth(135);


    buttonlayout->addWidget(sendButton);
    buttonlayout->addSpacing(10);
    buttonlayout->addWidget(clearReceiveButton);
    buttonlayout->addSpacing(10);
    buttonlayout->addWidget(clearSendButton);
    showdatalayout->addSpacing(15);
    showdatalayout->addLayout(buttonlayout);

    serialconfigwidget->setObjectName("serialconfigwidget");
    serialconfigwidget->setStyleSheet(R"(
    QWidget#serialconfigwidget{
        background-color: qlineargradient(
            x1: 0, y1: 0, x2: 0, y2: 1,
            stop: 0 rgba(35, 45, 65, 240),   /* 顶部略亮蓝灰 */
            stop: 1 rgba(20, 30, 50, 240)    /* 底部更暗 */
        );
        border: 1px solid rgba(255, 255, 255, 0.08);      /* 柔和外边框 */
        border-radius: 12px;
        padding: 2px;

    }
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
              width: 100px;  /* 设置最小宽度 */
              height: 25px;  /* 设置最小高度 */
          }
          QPushButton:pressed {
              background-color: rgba(100, 120, 200, 50);
          }
    )");

    showdatawidget->setObjectName("showdatawidget");
    showdatawidget->setStyleSheet(R"(
    QWidget#showdatawidget{
        background-color: qlineargradient(
            x1: 0, y1: 0, x2: 0, y2: 1,
            stop: 0 rgba(35, 45, 65, 240),   /* 顶部略亮蓝灰 */
            stop: 1 rgba(20, 30, 50, 240)    /* 底部更暗 */
        );
        border: 1px solid rgba(255, 255, 255, 0.08);      /* 柔和外边框 */
        border-radius: 12px;
        padding: 2px;

    }
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
          width: 100px;  /* 设置最小宽度 */
          height: 25px;  /* 设置最小高度 */
      }
      QPushButton:pressed {
          background-color: rgba(100, 120, 200, 50);
      }
    )");

    QString inputStyle = R"(
        QLineEdit, QTextEdit {
            background-color: #2E3A4B;  /* 浅色背景，与背景有区分 */
            color: white;  /* 文字颜色为白色 */
            border: 2px solid #3C4A5C;  /* 边缘颜色 */
            border-radius: 8px;  /* 圆角 */
            padding: 2px;  /* 内边距 */
            font-size: 14px;  /* 设置字体大小 */
        }

        QLineEdit:focus, QTextEdit:focus {
            border: 2px solid #4A76A8;  /* 获取焦点时，边缘颜色发光 */
        }

        QLineEdit::placeholder, QTextEdit::placeholder {
            color: #A8B1C1;  /* 设置占位符文字颜色 */
        }
    )";

    // Apply to your input fields
    receiveTextEdit->setStyleSheet(inputStyle);
    sendTextEdit->setStyleSheet(inputStyle);

    //右侧状态显示布局
    QWidget *statuscheckwidget = new QWidget();
    statuscheckwidget->setFixedWidth(200);
    statuscheckwidget->setObjectName("statuscheckwidget");
    statuscheckwidget->setStyleSheet(R"(
    QWidget#statuscheckwidget{
        background-color: qlineargradient(
            x1: 0, y1: 0, x2: 0, y2: 1,
            stop: 0 rgba(35, 45, 65, 240),   /* 顶部略亮蓝灰 */
            stop: 1 rgba(20, 30, 50, 240)    /* 底部更暗 */
        );
        border: 1px solid rgba(255, 255, 255, 0.08);      /* 柔和外边框 */
        border-radius: 12px;
        padding: 2px;

    }
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
          width: 100px;  /* 设置最小宽度 */
          height: 25px;  /* 设置最小高度 */
      }
      QPushButton:pressed {
          background-color: rgba(100, 120, 200, 50);
      }
    )");

    QVBoxLayout *statusLayout = new QVBoxLayout(statuscheckwidget);
    statusLayout->setContentsMargins(10,30,10,30);

    // 创建frame用来显示状态信息
    voltageStatusFrame = new QFrame();
    currentStatusFrame = new QFrame();
    temperatureStatusFrame = new QFrame();
    powerOutputStatusFrame = new QFrame();
    reverseReflectionStatusFrame = new QFrame();

    //设置默认状态
    setStatus(voltageStatusFrame, false);
    setStatus(currentStatusFrame, false);
    setStatus(temperatureStatusFrame, false);
    setStatus(powerOutputStatusFrame, false);
    setStatus(reverseReflectionStatusFrame, false);

    // 设置布局
    QLabel *voltagelabel = new QLabel(tr("电压："));
    voltagelabel->setStyleSheet("background-color: transparent;");
    statusLayout->addWidget(voltagelabel);
    statusLayout->addWidget(voltageStatusFrame);

    statusLayout->addSpacing(15);

    QLabel *currentlabel = new QLabel(tr("电流："));
    currentlabel->setStyleSheet("background-color: transparent;");
    statusLayout->addWidget(currentlabel);
    statusLayout->addWidget(currentStatusFrame);

    statusLayout->addSpacing(15);

    QLabel *temperaturelabel = new QLabel(tr("温度："));
    temperaturelabel->setStyleSheet("background-color: transparent;");
    statusLayout->addWidget(temperaturelabel);
    statusLayout->addWidget(temperatureStatusFrame);

    statusLayout->addSpacing(15);

    QLabel *outputpowerlabel = new QLabel(tr("输出功率："));
    outputpowerlabel->setStyleSheet("background-color: transparent;");
    statusLayout->addWidget(outputpowerlabel);
    statusLayout->addWidget(powerOutputStatusFrame);

    statusLayout->addSpacing(15);

    QLabel *reversestandingwavelabel = new QLabel(tr("反向驻波："));
    reversestandingwavelabel->setStyleSheet("background-color: transparent;");
    statusLayout->addWidget(reversestandingwavelabel);
    statusLayout->addWidget(reverseReflectionStatusFrame);

    //添加所有布局到总布局
    mainLayout->addWidget(serialconfigwidget);
    mainLayout->addSpacing(15);
    mainLayout->addWidget(showdatawidget);
    mainLayout->addSpacing(15);
    mainLayout->addWidget(statuscheckwidget);

    // 连接信号与槽
    connect(connectButton, &QPushButton::clicked, this, &MaintenanceWidget::onConnectSerialPort);
    connect(sendButton, &QPushButton::clicked, this, &MaintenanceWidget::onSendData);
    connect(clearReceiveButton, &QPushButton::clicked, this, &MaintenanceWidget::onClearReceive);
    connect(clearSendButton, &QPushButton::clicked, this, &MaintenanceWidget::onClearSend);
    connect(serialManager, &SerialManager::dataReceived, this, &MaintenanceWidget::onReceiveData);

    //UDP监听数据
//    udpManager->startListening(5555);    // 启动监听UDP数据（监听某个端口，例如5555）

    // 连接信号，处理接收到的数据
//    connect(udpManager, &UdpManager::dataReceived, this, &MaintenanceWidget::onDataReceived);
}

MaintenanceWidget::~MaintenanceWidget()
{
    delete serialManager;
}

void MaintenanceWidget::onConnectSerialPort()
{
    // 判断当前按钮文本是“连接”还是“断开”
    if (connectButton->text() == tr("连接串口")) {
        // 执行连接串口操作
        QString portName = portComboBox->currentText();
        int baudRate = baudRateComboBox->currentText().toInt();
        int dataBits = dataBitsComboBox->currentIndex() + 5;
        int parity = parityComboBox->currentIndex();
        int stopBits = stopBitsComboBox->currentIndex();

        bool success = serialManager->openPort(portName, baudRate, dataBits, parity, stopBits);
        if (success) {
            qDebug() << "串口连接成功！";
            connectButton->setText(tr("断开连接"));  // 更改按钮文本为“断开连接”
        } else {
            qDebug() << "无法打开串口！";
        }
    } else {
        // 执行断开串口操作
        serialManager->closePort();
        qDebug() << "串口已断开";
        connectButton->setText(tr("连接串口"));  // 更改按钮文本为“连接串口”
        connectButton->setIcon(QIcon(":/image/icons8-disconnect.png"));
    }
}


void MaintenanceWidget::onSendData()
{
    QByteArray data = sendTextEdit->toPlainText().toUtf8();
    serialManager->sendData(data);
    qDebug() << "发送数据：" << data;
}

void MaintenanceWidget::onReceiveData(const QByteArray &data)
{
    qDebug() << "接收到的数据：" << data.toHex();

    // 数据长度判断，至少需要帧头（1字节）和指令类型（1字节）
    if (data.size() < 4) {
        qDebug() << "数据不完整，无法解析";
        return;
    }

    // 获取当前时间戳
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

    // 判断帧头是否为 0x55 和指令类型是否为 0xBB
    if (static_cast<unsigned char>(data[0]) == 0x55 && static_cast<unsigned char>(data[1]) == 0xBB) {
        qDebug() << "数据帧头和指令类型匹配，开始解析数据...";

          parseReceivedData(data);

    }

    // 判断帧头（字节 0）是否为 0x55 和指令类型（字节 1）是否为 0xAA
    if (static_cast<unsigned char>(data[0]) == 0x55 && static_cast<unsigned char>(data[1]) == 0xAA  && data.size() == 4) {
        qDebug() << "数据帧头和指令类型匹配，开始解析数据...";
        parseAttenuationResponse(data);
    }

    else {
        qDebug() << "数据帧头或指令类型不匹配，忽略数据";
    }

       // 将字节数据转换为十六进制字符串
       QString dataString = data.toHex(' ');  // 将字节数据转换为十六进制字符串

       // 拼接时间戳和接收到的数据
       QString displayString = QString("[%1] %2").arg(timestamp, dataString);

       // 将数据添加到接收区
       receiveTextEdit->append(displayString);  // 追加数据到接收区

}

void MaintenanceWidget::onClearReceive()
{
    receiveTextEdit->clear();
}

void MaintenanceWidget::onClearSend()
{
    sendTextEdit->clear();
}

void MaintenanceWidget::setStatus(QFrame *frame, bool isNormal)
{
    if (isNormal) {
        frame->setStyleSheet("background-color: green;");
    } else {
        frame->setStyleSheet("background-color: red;");
    }
}

void MaintenanceWidget::parseReceivedData(const QByteArray &data)
{
    // 数据格式是: [0x55 0xBB | 电压状态 | 电流状态 | 温度状态 | 输出功率状态 | 反向驻波状态]

    if (data.size() < 7) {
        qDebug() << "数据长度不够，无法解析";
        return;
    }

    uint8_t voltageStatus = data[2];    // 电压状态
    uint8_t currentStatus = data[3];    // 电流状态
    uint8_t temperatureStatus = data[4]; // 温度状态
    uint8_t outputPowerStatus = data[5]; // 输出功率状态
    uint8_t reverseReflectionStatus = data[6]; // 反向驻波状态

    // 更新 UI 状态
    setStatus(voltageStatusFrame, voltageStatus == 1);    // 电压状态
    setStatus(currentStatusFrame, currentStatus == 1);    // 电流状态
    setStatus(temperatureStatusFrame, temperatureStatus == 1); // 温度状态
    setStatus(powerOutputStatusFrame, outputPowerStatus == 1); // 输出功率状态
    setStatus(reverseReflectionStatusFrame, reverseReflectionStatus == 1); // 反向驻波状态
}

void MaintenanceWidget::parseAttenuationResponse(const QByteArray &data)
{
        // 获取设置结果（字节 2）
        unsigned char result = static_cast<unsigned char>(data[2]);

        // 获取时间戳
        QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

        // 将接收到的设置结果转换为字符串
        QString resultString = QString(result == 0x00 ? "成功" : "失败");

        // 拼接时间戳和衰减设置应答结果
        QString displayString = QString("[%1] 衰减设置结果: %2").arg(timestamp, resultString);

        // 将数据添加到接收区
        receiveTextEdit->append(displayString);
}

void MaintenanceWidget::onDataReceived(const QByteArray &data)
{
    // 这里是接收到数据后处理的代码
    // 比如，可以将接收到的十六进制数据转换成字符串并显示到界面上
    QString dataString = QString::fromUtf8(data);
    qDebug() << "接收到的数据：" << dataString;
}


