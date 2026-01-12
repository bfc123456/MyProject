#include "customzoomer.h"

CustomZoomer::CustomZoomer(QwtPlotCanvas* canvas) : QwtPlotZoomer(canvas){
    setZoomBase();  //记录初始视图为基准
    setRubberBand(QwtPicker::RectRubberBand);
    setRubberBandPen(QPen(Qt::green, 1 , Qt::DashLine));
    setTrackerMode(QwtPicker::AlwaysOff);   //关闭坐标提示
}

//事件过滤：处理双击
bool CustomZoomer::eventFilter(QObject * obj, QEvent * event){
    //左键双击，未达最大级别则放大一级
    if(event->type() == QEvent::MouseButtonDblClick){
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        if(mouseEvent->button() == Qt::LeftButton && zoomRectIndex() < MAX_ZOOM_LEVEL){
            zoom(1);
            return true;
        }
    }
    //右键单击：重置到基准视图
    else if (event->type() == QEvent::MouseButtonPress){
        QMouseEvent* mouseEvent = static_cast<QMouseEvent * >(event);
        if(mouseEvent->button() == Qt::RightButton){
            zoom(0);
            return true;
        }
    }
    return QwtPlotZoomer::eventFilter(obj , event);
}
