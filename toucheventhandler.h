
#ifndef TOUCHEVENTHANDLER_H
#define TOUCHEVENTHANDLER_H

#include <QObject>
#include <QEvent>
#include <QTouchEvent>
#include <QDebug>

class TouchEventHandler : public QObject
{
    Q_OBJECT

public:
    explicit TouchEventHandler(QObject *parent = nullptr);     //防止隐性类型转换
    ~ TouchEventHandler() = default;    //默认析构函数

    void handleTouchEvent(QTouchEvent *touchEvent);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;  //明确重写基类虚函数
};

#endif // TOUCHEVENTHANDLER_H
