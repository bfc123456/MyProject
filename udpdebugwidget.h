#ifndef UDPDEBUGWIDGET_H
#define UDPDEBUGWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "modernwaveplot.h"
#include <qwt_plot_curve.h>
#include <qwt_series_data.h>
#include <QUdpSocket>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QElapsedTimer>
#include "FramelessWindow.h"
#include "customkeyboard.h"

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
