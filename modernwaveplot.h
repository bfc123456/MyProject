#ifndef MODERNWAVEPLOT_H
#define MODERNWAVEPLOT_H

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_picker.h>
#include <QVector>
#include <QPointF>
#include "customyscaledraw.h" // 确保您有这个文件，没有的话用我上一条回复的

class ModernWavePlot : public QwtPlot {
    Q_OBJECT

public:
    explicit ModernWavePlot(QWidget *parent = nullptr);

    // 核心：增量更新
    void updateIncrementalData(const QVector<QPointF>& data);

    // 兼容旧接口
    void clearSimpleData();
    void clearWaveform();
    void setSimpleData(const QVector<QPointF>& data);

    // 样式与设置
    void initPlotStyle();
    void setLineColor(const QColor& color);
    void setFillColor(const QColor& fill, double baseline = 25.0);
    void setLiveMode(bool on);

private:
    void initPlotInteraction();

private slots:
    void showPointValue(const QPointF &pos);

private:
    QwtPlotCurve *m_curve;
    QwtPlotMarker *m_valueMarker;
    QwtPlotPicker *m_picker;

    // 显示缓存，保证点不丢
    QVector<QPointF> m_displayBuffer;

    bool m_liveMode = true;
};

#endif // MODERNWAVEPLOT_H
