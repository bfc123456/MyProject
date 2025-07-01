#include "customcombobox.h"
#include <QStyledItemDelegate>
#include <QListView>
#include <QPainter>

// 构造函数，接受自定义的选项高度
CustomComboBox::CustomComboBox(int itemHeight, QWidget *parent)
    : QComboBox(parent), m_itemHeight(itemHeight)
{
    // 设置下拉框的 itemDelegate，默认使用 QStyledItemDelegate
    QListView *view = new QListView(this);
    view->setItemDelegate(new QStyledItemDelegate(this));
    this->setView(view);
}

// 重写 showPopup 来控制下拉区域每项的高度
void CustomComboBox::showPopup()
{
    // 使用 QComboBox 内部的 View 设置每项的高度
    QListView *view = qobject_cast<QListView *>(this->view());
    if (view) {
        view->setFixedHeight(m_itemHeight * this->count()); // 设置下拉列表总高
        view->setStyleSheet(QString("QListView::item { min-height: %1px; }").arg(m_itemHeight));  // 设置每项高度
    }
    QComboBox::showPopup();
}
