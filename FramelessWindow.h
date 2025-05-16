
#ifndef FRAMELESSWINDOW_H
#define FRAMELESSWINDOW_H

#pragma once    //头文件保护指令，确保只包含一次

#include <QWidget>
#include <QShowEvent>

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
