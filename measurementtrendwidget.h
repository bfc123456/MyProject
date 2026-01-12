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
#include "framelesswindow.h"
#include <qwt_legend.h>
#include "global.h"

/**
 * @class TimeScaleDraw
 * @brief 趋势图 X 轴自定义时间刻度绘制器
 *
 * 将 X 轴原始值视为自 Unix 纪元起的毫秒时间戳，并格式化为 "MM-dd hh:mm"。
 * 典型用法：设置给 QwtPlot 的 xBottom 轴以获得友好的时间标签。
 *
 * 注意：
 * - 期望 value 为毫秒级时间戳（QDateTime::fromMSecsSinceEpoch）。
 * - 若业务使用秒/纳秒，需要在外部做单位换算。
 */

class TimeScaleDraw : public QwtScaleDraw {
public:
    explicit TimeScaleDraw() = default;

    QwtText label(double value) const override {
        QDateTime dt = QDateTime::fromMSecsSinceEpoch(static_cast<qint64>(value));
        return dt.toString("MM-dd hh:mm");
    }
};

/**
 * @class TrendChart
 * @brief 测量趋势图控件（基于 QwtPlot）
 *
 * 展示历史/当前的多条曲线，包括：舒张压(Dia)、收缩压(Sys)、平均压(Avg)、
 * 心率(HR) 等。支持动态更新数据与多语言重载。
 *
 * 责任边界：
 * - 仅负责曲线的绘制、图例显示与坐标轴配置；
 * - 数据加载与持久化由上层（MeasurementTrendWidget）负责；
 * - setData() 接收外部整理好的 MeasurementData 序列。
 *
 * 使用步骤：
 * 1) new TrendChart(parent)
 * 2) setData(history) 传入数据
 * 3) updatePlot() 刷新绘制
 * 4) retranslate() 语言切换时更新文本
 */

class TrendChart : public QwtPlot {
    Q_OBJECT
public:
    explicit TrendChart(QWidget *parent = nullptr);
    void setData(const QVector<MeasurementData> &data);
    void updatePlot();
    void retranslate();

private:
    void setupCurves();

    QVector<MeasurementData> m_data;
    QwtPlotCurve *curveDia;
    QwtPlotCurve *curveSys;
    QwtPlotCurve *curveAvg;
    QwtPlotCurve *curveHR;
    QwtLegend    *m_legend;
};

/**
 * @class MeasurementTrendWidget
 * @brief 测量趋势主界面（无边框窗口）
 *
 * 该窗口聚合了：当前测量结果卡片、历史趋势图（TrendChart）、功能按钮区
 *（返回/主页/导出/关机）及设置窗口入口等。负责：
 * - 加载/保存历史数据；
 * - 展示当前测量摘要与趋势曲线；
 * - 提供导出数据、再次测量、打开设置等操作；
 * - 响应多语言切换，统一刷新文案。
 *
 * 信号：
 * - trendReturnMeasure()：返回测量流程
 * - openSettingsWindowResult()：打开设置面板
 * - closeAllWindow()：关闭所有相关窗口（如退出应用前清理）
 *
 * 典型调用：
 *   MeasurementTrendWidget w(currentResult);
 *   w.show();
 *   w.updateMeasurementResult(newResult);  // 新增结果时刷新卡片与趋势
 */

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

protected:
    void changeEvent(QEvent *event) override;

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
//    QLabel* titleLabel{nullptr};
    QPushButton *backButton;
    QPushButton *homeButton;
    QPushButton *exportButton;
    QPushButton *shutdownButton;
    QLabel *summaryTitleLabel;

    //界面指针
    SettingsWidget *settingsWidgetResult;
    QString fetchSensorIds() const; //保障不修改对象状态
    TrendChart       *m_chart;
    QString           m_sensorId;
    QVector<MeasurementData> m_historyData;
    QVector<QLabel*> m_cardTitleLabels;            // 卡片标题集合
    QVector<QLabel*> m_cardUnitLabels;             // 卡片单位集合

};

#endif // MEASUREMENTTRENDWIDGET_H
