/******************************************************************************
 * 文件名：cardiacoutputdialog.h
 * 功能描述：提供心输出量输入的对话框界面，允许用户输入心输出量（CO）。
 *           根据血压和平均值自动生成界面，并支持自定义数字键盘。
 * 版本：v1.0.0
 * 作者：
 * 时间：2025/12/25
 * 版本控制：v1.0.0 - 初始版本，提供基本的心输出量输入功能
 * 备注：该文件包含一个对话框，用于输入和显示心输出量。它通过继承CloseOnlyWindow，
 *       提供了一个只包含关闭按钮的对话框，并实现了屏幕分辨率适配。
 ******************************************************************************/

#ifndef CARDIACOUTPUTDIALOG_H
#define CARDIACOUTPUTDIALOG_H

//1) Project Headers
#include "customkeyboard.h"
#include "closeonlywindow.h"

//2) Qt Headers
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

class CardiacOutputDialog : public CloseOnlyWindow {
    Q_OBJECT

public:

    explicit CardiacOutputDialog(QString strBpValue, QString strAvgValue, QWidget *pParent = nullptr);
    ~CardiacOutputDialog();
    QString GetCOValue() const;

private:
    QLineEdit *m_pCoEdit; /**< 用于输入和显示心输出量的输入框。 */
    CustomKeyboard* m_pCurrentKeyboard = nullptr; /**< 当前绑定的自定义数字键盘。 */

    float m_pScaleX; /**< 屏幕X轴缩放比例，确保界面适配不同分辨率。 */
    float m_pScaleY; /**< 屏幕Y轴缩放比例，确保界面适配不同分辨率。 */

    // 按钮和标签控件
    QPushButton *m_pBackBtn; /**< 返回按钮 */
    QPushButton *m_pSaveBtn; /**< 保存按钮 */
    QPushButton *m_pCloseBtn; /**< 关闭按钮 */

    QLabel *m_pTitleLbl; /**< 标题标签 */
    QLabel *m_pBpLbl; /**< 血压标签 */
    QLabel *m_pAvgLbl; /**< 平均值标签 */
    QLabel *m_pUnitLbl; /**< 单位标签 */
    QLabel *m_pCoTitleLbl; /**< 心输出量标题标签 */
    QLabel *m_pCoUnitLbl; /**< 心输出量单位标签 */
};

#endif // CARDIACOUTPUTDIALOG_H
