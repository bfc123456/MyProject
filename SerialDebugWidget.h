
#ifndef SERIALDEBUGWIDGET_H
#define SERIALDEBUGWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QComboBox>
#include <QTextEdit>
#include <QLineEdit>
#include <QLabel>
#include "serialmanager.h"
#include "udpmanager.h"
#include "framelesswindow.h"
#include "customkeyboard.h"

/**
 * @class SerialDebugWidget
 * @brief 串口调试界面
 *
 * 本类提供一个图形化的串口调试工具，供工程师或维护人员在设备维护、
 * 开发和测试过程中使用。用户可以通过界面选择串口端口、设置波特率、
 * 发送/接收数据包，实时观察设备的响应。
 *
 * 功能特点：
 * - 提供串口端口选择、波特率、校验位、停止位等参数设置
 * - 支持打开/关闭串口，实时显示接收到的数据
 * - 提供文本输入框，支持手动发送数据
 * - 接收区支持文本和十六进制显示，方便查看原始数据
 * - 可与日志系统 (MedicalLogger) 联动，记录调试过程
 *
 * 使用场景：
 * - 工程调试阶段：验证设备串口通信是否正常
 * - 维护场景：快速定位设备与上位机之间的通信问题
 * - 开发测试：模拟下位机/上位机发送命令，验证协议正确性
 *
 * @note
 * - 与 UdpDebugWidget 风格保持一致，便于统一维护
 * - 不参与正常业务逻辑，仅作为调试工具使用
 */

class SerialDebugWidget : public FramelessWindow
{
    Q_OBJECT

public:
    explicit SerialDebugWidget(QWidget *parent = nullptr);
    ~SerialDebugWidget();

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

#endif // SERIALDEBUGWIDGET_H
