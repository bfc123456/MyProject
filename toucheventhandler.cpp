#include "toucheventhandler.h"

TouchEventHandler::TouchEventHandler(QObject *parent): QObject(parent)
{

}

void TouchEventHandler::handleTouchEvent(QTouchEvent *touchEvent)
{
    for(const QTouchEvent::TouchPoint &point : touchEvent->touchPoints()){ //遍历QTouchEvent对象中的所有触摸点的信息赋值给point变量
        if(point.state() == Qt::TouchPointPressed){
            qDebug()<<"Touch pressed at"<< point.pos();
        }else if(point.state() == Qt::TouchPointMoved){
            qDebug()<<"Touch pressed at"<< point.pos();
        }else if(point.state() ==Qt::TouchPointReleased){
            qDebug()<<"Touch released at"<< point.pos();
        }
    }
}

bool TouchEventHandler::eventFilter(QObject *watched, QEvent *event){   //指向触发事件的指针监听对象，指向QEvent类型的指针
    if(event->type() == QEvent::TouchBegin || event->type() == QEvent::TouchUpdate || event->type() ==QEvent::TouchEnd){
        QTouchEvent *touchEvent = static_cast<QTouchEvent*>(event); //将事件强制转换为触控事件
        handleTouchEvent(touchEvent); //处理触控事件
        return true; //事件被处理
    }
    return QObject::eventFilter(watched,event);  //其他事件交给默认处理
}
