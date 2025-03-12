#ifndef VIRTUALKEYBOARD_H
#define VIRTUALKEYBOARD_H

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>

class VirtualKeyboard : public QWidget {
    Q_OBJECT
public:
    VirtualKeyboard(QWidget *parent = nullptr);
    void showKeyboard();  // 显示虚拟键盘
    void hideKeyboard();  // 隐藏虚拟键盘
};

#endif // VIRTUALKEYBOARD_H
