
#ifndef TOUCHEVENTHANDLER_H
#define TOUCHEVENTHANDLER_H

#include <QObject>
#include <QEvent>
#include <QTouchEvent>
#include <QDebug>

/**
 * @class TouchEventHandler
 * @brief 触控事件处理器
 *
 * 本类继承自 QObject，主要用于全局安装在 QApplication 上，
 * 将触摸事件和鼠标事件进行统一处理，从而在触控设备上保证界面操作流畅。
 *
 * 功能职责：
 * - 过滤和拦截应用中的输入事件（eventFilter）
 * - 将触控事件（TouchEvent）映射为鼠标事件，保证界面控件可正常使用
 * - 在没有触控支持的环境中，模拟触摸行为（如拖动、点击）
 * - 作为全局事件过滤器安装到 QApplication，使整个应用受益
 *
 * 使用场景：
 * - 应用运行在带触摸屏的医疗设备上
 * - 用户通过手指点击、拖动，而不是传统鼠标操作
 * - 确保自定义控件（如 FramelessWindow、CircularProgressBar）能在触屏环境下正常交互
 *
 * @note
 * - 通常在 main.cpp 中通过 `a.installEventFilter(new TouchEventHandler());` 注册
 * - 本类不涉及业务逻辑，仅负责事件层的输入统一
 * - 建议与日志系统结合，记录触控异常情况
 */

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
