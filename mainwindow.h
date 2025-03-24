#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QLabel>
#include <QPixmap>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTextEdit>
#include <QSpacerItem>
#include <QFrame>
#include <QFormLayout>
#include <QSplitter>
#include <QTranslator>
#include <QElapsedTimer>
#include "databasemanager.h"
#include "serialportmanager.h"
#include "maintenance.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void changeLanguage(const QString &languageCode);
    void applyLanguageChange();

private slots:
    void onStartClicked();
    void onStopClicked();
    void onTimerTimeout();
    void onFinishMeasurement();
    void updateGraph();
    void onUploadDataClicked();
    void onHistoryButtonClicked();
    void resetMeasurementData();
    void onDataReceived(QByteArray data);
    void onErrorOccurred(QString errorMsg);
//    void on_pushButtonSend_clicked();
    void onSettingClicked();


private:
    Ui::MainWindow *ui;
    QPushButton *measureButton;
    QPushButton *uploadButton;
    QPushButton *historyButton;
    QPushButton *settingsButton;

    QwtPlot *plot;
    QwtPlotCurve *curve;
    QVector<double> xData;
    QVector<double> yData;
    QSerialPort *serialPort;
    QTimer *timer;
    int savedTime = 0;  // 用于存储暂停时的时间

    // UI 控件
    struct PressurePanel {
        QFrame *panel;
        QLabel *paramLabel;
        QLabel *unitLabel;
        QLabel *valueLabel;
    };

    PressurePanel diastolicPanel;
    PressurePanel systolicPanel;
    PressurePanel avgPanel;
    PressurePanel heartbeatPanel;

    QLabel *diastolicValue;
    QLabel *systolicValue;
    QLabel *avgValue;
    QLabel *heartbeatValue;

    bool isMeasuring = false;   // 是否正在测量
    int currentTime = 0;        // 当前时间（秒）
    double maxPressure = 25.0;  // 最大压力
    double minPressure = 6.0;  // 最小压力
    bool isPaused = false;      // 是否暂停

    QVector<QPointF> generateData();

    databasemanager dbManager;  // 声明数据库管理对象
    SerialPortManager *serialManager;   //声明串口管理对象
    QTranslator translator;//翻译器

    QElapsedTimer clickTimer; //连续点击记录器
    MaintenanceWidget  *hiddenWidget;//隐藏界面

    void changeEvent(QEvent *event) override;
    void showHiddenWidget();


};
#endif // MAINWINDOW_H
