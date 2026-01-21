#include "modernwaveplot.h"
#include <qwt_plot_canvas.h>
#include <qwt_plot_layout.h>
#include <qwt_scale_widget.h>
#include <qwt_symbol.h>
#include <cmath>

ModernWavePlot::ModernWavePlot(QWidget *parent)
    : QwtPlot(parent), m_curve(new QwtPlotCurve())
{
    initPlotStyle();
    initPlotInteraction();

    m_curve->setRenderHint(QwtPlotItem::RenderAntialiased);
    m_curve->setPen(QPen(QColor(100, 180, 255), 2));
    m_curve->attach(this);
}

void ModernWavePlot::initPlotStyle() {
    canvas()->setPalette(QPalette(Qt::black));
    plotLayout()->setCanvasMargin(0);

    QwtPlotGrid* grid = new QwtPlotGrid();
    grid->setPen(QPen(QColor(255, 255, 255, 30), 1, Qt::DotLine));
    grid->attach(this);

    // Y 轴设置 (白色)
    QwtScaleWidget *yAxis = axisWidget(QwtPlot::yLeft);
    if(yAxis) {
        QPalette pal = yAxis->palette();
        pal.setColor(QPalette::WindowText, Qt::white);
        pal.setColor(QPalette::Text, Qt::white);
        yAxis->setPalette(pal);

        QwtText yTitle("Frequency (MHz)");
        yTitle.setColor(Qt::white);
        yTitle.setFont(QFont("Arial", 12, QFont::Bold));
        setAxisTitle(QwtPlot::yLeft, yTitle);

        yAxis->setScaleDraw(new CustomYScaleDraw(0));
    }

    // X 轴设置 (白色)
    QwtScaleWidget *xAxis = axisWidget(QwtPlot::xBottom);
    if(xAxis) {
        QPalette pal = xAxis->palette();
        pal.setColor(QPalette::WindowText, Qt::white);
        pal.setColor(QPalette::Text, Qt::white);
        xAxis->setPalette(pal);

        QwtText xTitle("Time (ms)");
        xTitle.setColor(Qt::white);
        xTitle.setFont(QFont("Arial", 12, QFont::Bold));
        setAxisTitle(QwtPlot::xBottom, xTitle);
    }

    setAxisScale(QwtPlot::yLeft, 0, 50.0);
    setAxisScale(QwtPlot::xBottom, 0, 120.0);
    setAxisAutoScale(QwtPlot::yLeft, false);
}

// 【核心】：带 Y 轴自适应的增量更新
// modernwaveplot.cpp

void ModernWavePlot::updateIncrementalData(const QVector<QPointF>& data) {
    if (data.isEmpty()) return;

    // 1. 存数据
    m_displayBuffer.append(data);

    // 2. 内存清理
    if (m_displayBuffer.size() > 3000) {
        m_displayBuffer.remove(0, m_displayBuffer.size() - 3000);
    }

    // 3. X 轴自动跟随
    double lastX = data.last().x();
    setAxisScale(QwtPlot::xBottom, lastX - 120.0, lastX);

    // 4. Y 轴自适应逻辑 + 智能精度控制
    if (m_liveMode && !m_displayBuffer.isEmpty()) {
        double minY = 1e10;
        double maxY = -1e10;

        int startIndex = qMax(0, (int)m_displayBuffer.size() - 500);
        for (int i = startIndex; i < m_displayBuffer.size(); ++i) {
            double y = m_displayBuffer[i].y();
            if (y < minY) minY = y;
            if (y > maxY) maxY = y;
        }

        double margin = (maxY - minY) * 0.1;
        if (margin < 0.1) margin = 0.1;

        double finalMin = minY - margin;
        double finalMax = maxY + margin;
        double currentRange = finalMax - finalMin;

        // 智能决定小数位数
        int precision = 0;
        if (currentRange < 2.0) {
            precision = 2; // 量程极小，2位小数
        } else if (currentRange < 10.0) {
            precision = 1; // 量程较小，1位小数
        } else {
            precision = 0; // 量程大，整数
        }

        // 设置精度
        QwtScaleWidget *yAxis = axisWidget(QwtPlot::yLeft);
        if (yAxis) {
            CustomYScaleDraw *scaleDraw = dynamic_cast<CustomYScaleDraw*>(yAxis->scaleDraw());
            if (scaleDraw) {
                // 【核心修复】：直接调用 setDecimalDigits，它内部会自动 invalidateCache
                scaleDraw->setDecimalDigits(precision);
            }
        }

        setAxisScale(QwtPlot::yLeft, finalMin, finalMax);
        m_curve->setBaseline(finalMin);
    }

    // 5. 绘制
    m_curve->setSamples(m_displayBuffer);
    replot();
}

void ModernWavePlot::setSimpleData(const QVector<QPointF>& data) {
    m_displayBuffer = data;
    m_curve->setSamples(m_displayBuffer);
    if(!data.isEmpty()) {
        double lastX = data.last().x();
        setAxisScale(QwtPlot::xBottom, lastX - 120.0, lastX);
    }
    replot();
}

void ModernWavePlot::clearSimpleData() {
    clearWaveform();
}

void ModernWavePlot::clearWaveform() {
    m_displayBuffer.clear();
    m_curve->setSamples(m_displayBuffer);
    replot();
}

void ModernWavePlot::showPointValue(const QPointF &pos) {
    QString textStr = QString::number(pos.y(), 'f', 4) + " MHz";
    QwtText whiteLabel(textStr);
    whiteLabel.setColor(Qt::white);
    whiteLabel.setBackgroundBrush(QBrush(QColor(0, 0, 0, 120)));

    m_valueMarker->setLabel(whiteLabel);
    m_valueMarker->setValue(pos);
    m_valueMarker->show();
    replot();
}

void ModernWavePlot::initPlotInteraction() {
    m_picker = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft,
               QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOn, canvas());
    m_picker->setTrackerPen(QPen(Qt::white));

    m_valueMarker = new QwtPlotMarker();
    m_valueMarker->setLabelAlignment(Qt::AlignTop | Qt::AlignRight);
    m_valueMarker->attach(this);

    connect(m_picker, SIGNAL(selected(const QPointF &)), this, SLOT(showPointValue(const QPointF &)));
}

void ModernWavePlot::setLineColor(const QColor& color) {
    if (m_curve) {
        m_curve->setPen(QPen(color, 2));
        replot();
    }
}

void ModernWavePlot::setFillColor(const QColor& fill, double baseline) {
    if (m_curve) {
        m_curve->setBrush(QBrush(fill));
        m_curve->setBaseline(baseline);
        replot();
    }
}

void ModernWavePlot::setLiveMode(bool on) {
    m_liveMode = on;
    replot();
}
