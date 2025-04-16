#include "customdateedit.h"
#include <QApplication>
#include <QDebug>

CustomDateEdit::CustomDateEdit(QWidget *parent)
    : QDateEdit(parent)
{
    setCalendarPopup(true);
    // 注册事件过滤器
    installEventFilter(this);
}

bool CustomDateEdit::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::Show) {
        // 检查是否是 QCalendarWidget 被弹出
        QCalendarWidget *calendar = this->calendarWidget();
        if (calendar && calendar->isVisible()) {
            QPoint belowPos = mapToGlobal(QPoint(0, height()));
            calendar->move(belowPos);
        }
    }
    return QDateEdit::eventFilter(watched, event);
}
