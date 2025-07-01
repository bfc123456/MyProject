// measurementtrendwidget.h
#ifndef MEASUREMENTTRENDWIDGET_H
#define MEASUREMENTTRENDWIDGET_H

#pragma once

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_scale_draw.h>
#include <QVector>
#include <QDateTime>
#include "measurementdata.h"
#include <QLabel>
#include <QPushButton>
#include "settingswidget.h"
#include "FramelessWindow.h"

// 自定义时间刻度绘制器
class TimeScaleDraw : public QwtScaleDraw {
public:
    explicit TimeScaleDraw() = default;

    QwtText label(double value) const override {
        QDateTime dt = QDateTime::fromMSecsSinceEpoch(static_cast<qint64>(value));
        return dt.toString("MM-dd hh:mm");
    }
};

class TrendChart : public QwtPlot {
    Q_OBJECT
public:
    explicit TrendChart(QWidget *parent = nullptr);
    void setData(const QVector<MeasurementData> &data);
    void updatePlot();

private:
    void setupCurves();

    QVector<MeasurementData> m_data;
    QwtPlotCurve *curveDia;
    QwtPlotCurve *curveSys;
    QwtPlotCurve *curveAvg;
    QwtPlotCurve *curveHR;
};

// 测量趋势主界面类
class MeasurementTrendWidget : public FramelessWindow {
    Q_OBJECT
public:
    explicit MeasurementTrendWidget(const MeasurementData &currentResult, QWidget *parent = nullptr);
    void updateMeasurementResult(const MeasurementData &newResult);     // 声明更新数据的接口
    ~MeasurementTrendWidget();

signals:
    void trendReturnMeasure();
    void openSettingsWindowResult();
    void closeAllWindow();

//protected:
//    void changeEvent(QEvent *event) override;

private slots:
    void onStartNewMeasurement();
    void closeSettingsWidget();
    void shutdownApp();
    void exportData();

private:
    // 界面组件创建函数
    QWidget* createTitleWidget();
    QWidget* createCardWidget(int dataIndex, const MeasurementData &result);
    QWidget* createTrendChartWidget();
    QWidget* createButtonsWidget();

    // 数据处理函数
    void loadHistoryData();
    void saveCurrentResult();
    void updateMeasurementStatus();
    void retranslateUi();

    // 界面布局变量
    float scaleX;
    float scaleY;
    QPixmap m_icons[4];
    QLinearGradient m_titleGrad;
    QLinearGradient m_btnGrads[2];
    MeasurementData m_currentResult;
    TrendChart* m_trendChart{nullptr};
    QLabel* titleLabel{nullptr};
    QPushButton *backButton;
    QPushButton *homeButton;
    QPushButton *exportButton;
    QPushButton *shutdownButton;

    //界面指针
    SettingsWidget *settingsWidgetResult;
    QString fetchSensorIds() const; //保障不修改对象状态
    TrendChart       *m_chart;
    QString           m_sensorId;
    QVector<MeasurementData> m_historyData;
};

#endif // MEASUREMENTTRENDWIDGET_H
