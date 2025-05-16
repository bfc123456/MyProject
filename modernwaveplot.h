
#pragma once

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <QVector>
#include <QPointF>

class ModernWavePlot : public QwtPlot {
    Q_OBJECT
public:
    explicit ModernWavePlot(QWidget *parent = nullptr);

    void setData(const QVector<QPointF>& data);
    void setLineColor(const QColor& color);
    void setFillColor(const QColor& fill, double baseline = 0);
    void setYRange(double min, double max);
    void setXRange(double min, double max);

private:
    QwtPlotCurve* curve;
};
