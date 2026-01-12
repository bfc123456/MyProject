#ifndef CUSTOMZOOMER_H
#define CUSTOMZOOMER_H

#include <qwt_plot_zoomer.h>
#include <qwt_plot_canvas.h>
#include <QMouseEvent>

class CustomZoomer : public QwtPlotZoomer
{
public:
    explicit CustomZoomer(QwtPlotCanvas* canvas);
    ~CustomZoomer() override = default;
protected:
          bool eventFilter(QObject* obj,QEvent* event) override;
private:
          static constexpr int MAX_ZOOM_LEVEL =  5;
};

#endif // CUSTOMZOOMER_H


