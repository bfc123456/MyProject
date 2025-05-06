#include "modernwaveplot.h"

#include <qwt_plot_canvas.h>
#include <qwt_scale_widget.h>
#include <QPalette>
#include <QBrush>
#include <QPen>
#include <QFont>
#include <cmath>
#include <qwt_plot_layout.h>

ModernWavePlot::ModernWavePlot(QWidget *parent)
    : QwtPlot(parent)
{
    this->setFrameStyle(QFrame::NoFrame);
    this->plotLayout()->setCanvasMargin(0);

    // 绘图区背景设置为黑色
    this->canvas()->setAutoFillBackground(true);
    this->canvas()->setAttribute(Qt::WA_OpaquePaintEvent, true);
    QPalette canvasPal;
    canvasPal.setColor(QPalette::Window, QColor(0, 0, 0));
    this->canvas()->setPalette(canvasPal);

    // 网格线
    QwtPlotGrid* grid = new QwtPlotGrid();
    grid->setPen(QPen(QColor(255, 255, 255, 15), 1, Qt::DotLine));
    grid->attach(this);

    // 曲线初始化
    curve = new QwtPlotCurve();
    curve->setRenderHint(QwtPlotItem::RenderAntialiased);
    curve->setCurveAttribute(QwtPlotCurve::Fitted, true);
    curve->attach(this);

    // 坐标轴样式
    this->axisWidget(QwtPlot::xBottom)->setFont(QFont("Arial", 9));
    this->axisWidget(QwtPlot::yLeft)->setFont(QFont("Arial", 9));

    // 设置坐标轴文字颜色为纯白色
    QwtScaleWidget* xAxis = this->axisWidget(QwtPlot::xBottom);
    QwtScaleWidget* yAxis = this->axisWidget(QwtPlot::yLeft);

    // 设置字体颜色为白
    QFont font = xAxis->font();
    font.setPointSize(10);
    xAxis->setFont(font);
    yAxis->setFont(font);

    // 设置坐标轴颜色为白色
    xAxis->setPalette(QPalette(Qt::white));
    yAxis->setPalette(QPalette(Qt::white));

    // 设置刻度文字颜色为白色
    QPalette axisPal;
    axisPal.setColor(QPalette::WindowText, Qt::white);  // 设置为纯白色
    xAxis->setPalette(axisPal);
    yAxis->setPalette(axisPal);

}

void ModernWavePlot::setData(const QVector<QPointF>& data) {
    curve->setSamples(data);
}

void ModernWavePlot::setLineColor(const QColor& color) {
    curve->setPen(QPen(color, 2));
}

void ModernWavePlot::setFillColor(const QColor& fill, double baseline) {
    curve->setBrush(QBrush(fill));
    curve->setBaseline(baseline);
}

void ModernWavePlot::setYRange(double min, double max) {
    this->setAxisScale(QwtPlot::yLeft, min, max);
}

void ModernWavePlot::setXRange(double min, double max) {
    this->setAxisScale(QwtPlot::xBottom, min, max);
}
