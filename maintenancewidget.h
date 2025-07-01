
#ifndef MAINTENANCEWIDGET_H
#define MAINTENANCEWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QComboBox>
#include <QTextEdit>
#include <QLineEdit>
#include <QLabel>
#include "serialmanager.h"
#include "udpmanager.h"
#include "FramelessWindow.h"
#include "customkeyboard.h"

class MaintenanceWidget : public FramelessWindow
{
    Q_OBJECT

public:
    explicit MaintenanceWidget(QWidget *parent = nullptr);
    ~MaintenanceWidget();

private slots:
    void onConnectSerialPort();  // 连接串口
    void onSendData();           // 发送数据
    void onClearReceive();       // 清空接收区
    void onClearSend();          // 清空发送区
    void onReceiveData(const QByteArray &data);        // 接收数据
    void onBtnCloseClicked();

private:

    SerialManager *serialManager;
    UdpManager *udpManager;

    QLabel *titleLabel;
    QLabel *portLabel;
    QLabel *baudRateLabel;
    QLabel *dataBitsLabel;
    QLabel *parityLabel;
    QLabel *stopBitsLabel;
    QLabel *flowControlLabel;
    QLabel *recivedatalable;
    QLabel *senddatalabel;
    QLabel *voltagelabel;
    QLabel *currentlabel;
    QLabel *temperaturelabel;
    QLabel *outputpowerlabel;
    QLabel *reversestandingwavelabel;
    QComboBox *portComboBox;
    QComboBox *baudRateComboBox;
    QComboBox *dataBitsComboBox;
    QComboBox *parityComboBox;
    QComboBox *stopBitsComboBox;
    QComboBox *flowControlComboBox;

    CustomKeyboard* currentKeyboard = nullptr;

    QPushButton *connectButton;
    QPushButton *sendButton;
    QPushButton *clearReceiveButton;
    QPushButton *clearSendButton;

    QTextEdit *receiveTextEdit;
    QLineEdit *sendTextEdit;

    // 状态指示框，用来显示每个监控项的状态
    QFrame *voltageStatusFrame;
    QFrame *currentStatusFrame;
    QFrame *temperatureStatusFrame;
    QFrame *powerOutputStatusFrame;
    QFrame *reverseReflectionStatusFrame;

    void setStatus(QFrame *frame, bool isNormal);

    //数据解析函数
    void parseReceivedData(const QByteArray &data); //发射通道状态查询应答指令解析
    void parseAttenuationResponse(const QByteArray &data);  //接收通道衰减应答指令解析

    void onDataReceived(const QByteArray &data);  //处理UDP监听的数据
    void changeEvent(QEvent *event) override;

    float scaleX;
    float scaleY;

};

#endif // MAINTENANCEWIDGET_H
