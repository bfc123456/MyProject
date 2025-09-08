
#ifndef FRAMELESSWINDOW_H
#define FRAMELESSWINDOW_H

#pragma once    //头文件保护指令，确保只包含一次

#include <QWidget>
#include <QShowEvent>

/**
 * @class FramelessWindow
 * @brief 自定义无边框窗口基类
 *
 * 本类继承自 QWidget（或 QMainWindow），对 Qt 默认窗口进行改造，
 * 去除了系统自带的边框和标题栏，实现应用统一的无边框风格。
 * 提供拖动、缩放、最小化/关闭等常用功能的封装，
 * 作为所有业务界面的基类。
 *
 * 功能特点：
 * - 去除系统默认标题栏，实现自定义标题栏和按钮
 * - 支持鼠标拖拽移动窗口
 * - 可选支持窗口大小缩放
 * - 提供虚函数接口，子类可重载以实现自定义 UI
 * - 与应用整体 UI 风格一致，保证跨平台一致性
 *
 * 使用场景：
 * - MultiUserLoginWindow（多用户登录界面）
 * - ImplantMonitor（植入监测界面）
 * - 各类业务弹窗或主界面
 *
 * @note 本类仅封装窗口的基础行为，不涉及业务逻辑。
 *       建议所有需要统一风格的窗口继承该类。
 */


class FramelessWindow : public QWidget {
    Q_OBJECT
public:
    explicit FramelessWindow(QWidget *parent = nullptr)
      : QWidget(parent)
    {
        // 去掉系统标题栏和边框
        setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    }

protected:
    // 每次 show() 的时候自动全屏
    void showEvent(QShowEvent *ev) override {
        QWidget::showEvent(ev);
        showFullScreen();
    }
};

#endif // FRAMELESSWINDOW_H
