#ifndef CUSTOMLINEEDIT_H
#define CUSTOMLINEEDIT_H

#include <QLineEdit>
#include <QFocusEvent>

class CustomLineEdit : public QLineEdit {
    Q_OBJECT
public:
    explicit CustomLineEdit(QWidget *parent = nullptr) : QLineEdit(parent) {}

protected:
    void focusInEvent(QFocusEvent *event) override {
        QLineEdit::focusInEvent(event);  // 调用基类的事件处理函数
        emit focusIn();  // 发射信号通知外部事件
    }

    void focusOutEvent(QFocusEvent *event) override {
        QLineEdit::focusOutEvent(event);  // 调用基类的事件处理函数
        emit focusOut();  // 发射信号通知外部事件
    }

signals:
    void focusIn();   // 焦点进入
    void focusOut();  // 焦点离开
};

#endif // CUSTOMLINEEDIT_H
