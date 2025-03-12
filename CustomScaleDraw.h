#ifndef CUSTOMSCALEDRAW_H
#define CUSTOMSCALEDRAW_H


#include <qwt_scale_draw.h>
#include <qwt_scale_map.h>
#include <QPainter>

#include <qwt_scale_draw.h>
#include <qwt_scale_map.h>
#include <qwt_scale_div.h>
#include <QPainter>

// **自定义 QwtScaleDraw 以修改刻度线颜色**
class CustomScaleDraw : public QwtScaleDraw {
public:
    explicit CustomScaleDraw(const QColor &color = Qt::white, int lineWidth = 2)
        : axisColor(color), penWidth(lineWidth) {}

    // **设置 X 轴和 Y 轴的刻度颜色**
    QPalette axisPalette;
    axisPalette.setColor(QPalette::Text, Qt::white);  // 设置刻度文本颜色为白色
    axisPalette.setColor(QPalette::Foreground, Qt::white);  // 设置刻度线颜色

    // **应用到 X 轴 & Y 轴**
    plot->axisWidget(QwtPlot::xBottom)->setPalette(axisPalette);
    plot->axisWidget(QwtPlot::yLeft)->setPalette(axisPalette);

    // **设置 X 轴 & Y 轴的轴线颜色**
    plot->setAxisPen(QwtPlot::xBottom, QPen(Qt::white, 2));  // X 轴白色
    plot->setAxisPen(QwtPlot::yLeft, QPen(Qt::white, 2));  // Y 轴白色


private:
    QColor axisColor;  // 坐标轴颜色
    int penWidth;      // 线宽
};


#endif // CUSTOMSCALEDRAW_H
