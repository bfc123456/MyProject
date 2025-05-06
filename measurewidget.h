#ifndef MEASUREWIDGET_H
#define MEASUREWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_symbol.h>
#include "settingswidget.h"
#include "modernwaveplot.h"
#include "measurementdata.h"
#include "customkeyboard.h"

class MeasureWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MeasureWidget(QWidget *parent = nullptr ,  const QString &sensorId = QString());
    ~MeasureWidget();

signals:
    void returnprepage();   //返回上一级界面
    void dataSaved(const MeasurementData &d);

private:
    ModernWavePlot *plot;
    QLabel *titleLabel;
    QLabel *idLabel;
    QLineEdit *systolicPressureEdit;
    QLineEdit *diastolicPressureEdit;
    QLineEdit *avgPressureEdit;
    QLineEdit *heartRateEdit;
    QLineEdit *coEdit;

    // 按钮
    QPushButton *zerobutton;
    QPushButton *thirtybutton;
    QPushButton *fortyfivebutton;
    QPushButton *ninetybutton;
    QPushButton *saveButton;
    QPushButton *clearButton;
    SettingsWidget *settingswidget;

    //波形显示
    QTimer *measurementTimer;
    QVector<QPointF> points;
    int currentTime = 0;  // 当前时间（秒）

    QLabel *sbpdata;
    QLabel *dbpdata;
    QLabel *meandata;
    QLabel *heartratedata;
    QLineEdit * referinputsbpdata;
    QLineEdit * referinputdbpdata;
    QLineEdit * refermeaninputdata;
    QLineEdit *remarklineedit;

    double minValue;     // 用于保存波形数据的最小值
    double maxValue;     // 用于保存波形数据的最大值
    double avgValue;     // 用于保存波形数据的平均值
    double heartRate;    // 用于保存波形数据的心率

    bool isYellow = false;  //按钮状态
    QString selectedAngle ;
    QString m_serial;
    CustomKeyboard *currentKeyboard;

private slots:
    void OpenSettingsRequested();
    void returnImplantmonitor();
    void openSaveConfirm();
    void onBtnClicked();
    void startMeasurement();
    void updateWaveform();
    double calculateHeartRate();
    void onSaveButtonClicked();
};

#endif // MEASUREWIDGET_H
