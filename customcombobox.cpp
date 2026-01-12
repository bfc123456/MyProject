
/********************************************************************************/
/* 文件名    : CustomComboBox.h                                                 */
/* 功能      : 自定义下拉框控件（支持自定义下拉项高度）                         */
/* 版本      : 1.0.0                                                            */
/* 作者      :                                                                  */
/* 日期      : 2025-12-29                                                       */
/* 说明      : 基于 QComboBox 扩展，用于统一 UI 风格并控制下拉项显示高度         */
/********************************************************************************/

//1) Project Headers
#include "customcombobox.h"
//2) Qt Headers
#include <QStyledItemDelegate>
#include <QListView>
#include <QPainter>

// 构造函数，接受自定义的选项高度
CustomComboBox::CustomComboBox(int itemHeight, QWidget *parent)
    : QComboBox(parent), m_iTemHeight(itemHeight)
{
    // 设置下拉框的 itemDelegate，默认使用 QStyledItemDelegate
    QListView *iView = new QListView(this);
    iView->setItemDelegate(new QStyledItemDelegate(this));
    this->setView(iView);
}

/***********************************************************************************************
 * FUNC    : showPopup
 * IN      : None
 * OUT     : None
 * RETURN  : void
 * AUTHOR  : 2025-12-29 Create by lxh for CustomComboBox class
 * NOTE    :
 *  - 重写 QComboBox::showPopup()，用于控制下拉列表中每一项的显示高度
 *  - 通过获取内部 QListView，对下拉列表的：
 *      1) 总高度（itemHeight × itemCount）
 *      2) 单项最小高度
 *    进行统一设置
 *  - 该方式常用于触屏 UI 场景，提升下拉选项的可点击性
 ************************************************************************************************/
void CustomComboBox::showPopup()
{
    // 使用 QComboBox 内部的 iView 设置每项的高度
    QListView *iView = qobject_cast<QListView *>(this->view());
    if (iView) {
        iView->setFixedHeight(m_iTemHeight * this->count()); // 设置下拉列表总高
        iView->setStyleSheet(QString("QListiView::item { min-height: %1px; }").arg(m_iTemHeight));  // 设置每项高度
    }
    QComboBox::showPopup();
}
