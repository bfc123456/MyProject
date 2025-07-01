#ifndef QSTYLEDITEMDELEGATE_H
#define QSTYLEDITEMDELEGATE_H

#include <QStyledItemDelegate>

class QStyledItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    // 构造函数接受高度参数
    explicit QStyledItemDelegate(int itemHeight, QObject *parent = nullptr);

    // 重写 sizeHint 来设置每项的高度
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    int m_itemHeight; // 存储每项的高度
};

#endif // QSTYLEDITEMDELEGATE_H
