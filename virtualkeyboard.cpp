#include "virtualkeyboard.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QDebug>

VirtualKeyboard::VirtualKeyboard(QWidget *parent) : QWidget(parent) {
    setFixedHeight(200);  // 设置虚拟键盘高度
    setStyleSheet("background-color: #CCCCCC;");  // 设置虚拟键盘背景颜色

    // 这里简单使用按钮作为虚拟键盘
    QVBoxLayout *keyboardLayout = new QVBoxLayout(this);
    for (int i = 0; i < 10; ++i) {
        QPushButton *button = new QPushButton(QString::number(i), this);
        keyboardLayout->addWidget(button);
    }
}

void VirtualKeyboard::showKeyboard() {
    setVisible(true);  // 设置虚拟键盘为可见
}

void VirtualKeyboard::hideKeyboard() {
    setVisible(false); // 设置虚拟键盘为不可见
}
