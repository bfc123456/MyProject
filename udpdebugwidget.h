#ifndef UDPDEBUGWIDGET_H
#define UDPDEBUGWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "ModernWaveplot.h"
#include <qwt_plot_curve.h>
#include <qwt_series_data.h>
#include <QUdpSocket>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QElapsedTimer>
#include "FramelessWindow.h"
#include "Customkeyboard.h"

/**
 * @class udpDebugWidget
 * @brief UDP 调试界面
 *
 * 本类提供一个图形化的 UDP 调试工具，允许用户在开发或维护阶段，
 * 直接进行 UDP 报文的收发测试，便于验证设备的网络通信是否正常。
 *
 * 功能职责：
 * - 设置本地 IP 与端口，用于监听下位机发送的数据
 * - 设置目标 IP 与端口，用于向设备发送测试报文
 * - 提供文本框，支持手动输入并发送报文（支持 ASCII/十六进制）
 * - 实时显示接收到的 UDP 报文（支持十六进制和文本格式）
 * - 显示当前 UDP 通信状态（是否已绑定、是否正在监听）
 * - 可与 MedicalLogger 联动，记录调试数据与操作行为
 *
 * 使用场景：
 * - 工程开发阶段：调试设备的 UDP 协议是否正确
 * - 维护场景：现场排查设备网络通信问题
 * - 联调测试：模拟上位机/下位机收发数据包
 *
 * @note
 * - 与 SerialDebugWidget 一致，属于调试工具类，不参与正常业务逻辑
 * - 内部可复用 UdpManager 提供的通信接口
 * - 推荐和日志系统结合，记录所有调试收发的数据帧
 */

class udpDebugWidget : public FramelessWindow{
    Q_OBJECT

public:
    udpDebugWidget(QWidget *parent = nullptr);
    ~ udpDebugWidget() = default;

private:
    //控件部分代码
    QLineEdit *ipLineEdit;
    QLineEdit *portLineEdit;
    QLabel *dataLabel1;
    QLabel *dataLabel2;
    QLabel *dataLabel3;
    QLabel *dataLabel4;
    QPushButton *exportBtn;
    QPushButton *importBtn;
    QPushButton *connectBtn;

    QUdpSocket *udpSocket;

    ModernWavePlot *plot1;
    ModernWavePlot *plot2;

    QwtPlotCurve *curve1 = nullptr;
    QwtPlotCurve *curve2 = nullptr;

    qint64 customTimeCounter = 0;  // 自增时间戳计数器

    QVector<QPointF> plotData1;
    QVector<QPointF> plotData2;

    CustomKeyboard *currentKeyboard = nullptr;

    // 存储每一帧的时间戳 + 6 个 float字
    QVector<QPair<qint64, QByteArray>> rawDataLog;


    //连接信号与槽
    void connectSignals();
    bool isVaildIP(const QString &ip);
    bool isVaildPort(const QString &port);
    void onConnectBtnClicked();

    //更新plot
    QElapsedTimer timer; //用于更高精度时间的计时
    void updatePlot1(float value, qint64 timestamp);
    void updatePlot2(float value, qint64 timestamp);
    float parseBigEndianFloat(const QByteArray& data, int offset);

    float scaleX;
    float scaleY;


private slots:
    //接收UDP发送的数据
    void ProcessReceivedData();
    //导出数据到CSV
    void exportHexToCSV(const QVector<QPair<qint64, QByteArray>> &dataList, const QString &filePath);   //导出数据
    QVector<QPointF>  importHexToPlot1(const QString &filePath);    //  导入数据到波线图
    QVector<QPointF>  importHexToPlot2(const QString &filePath);
    void onImportButtonClicked();  // 新的槽函数，用来处理点击事件
    void onBtnCloseClicked();
    void handleSocketError(QAbstractSocket::SocketError socketError);  // 声明函数


};

#endif // UDPDEBUGWIDGET_H
