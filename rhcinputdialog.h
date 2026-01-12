/******************************************************************************
 * 文件名：rhcinputdialog.h
 * 功能描述：提供RHC输入的对话框界面，允许用户输入RA、RV、PA、PCWP 等多组参数。
 * 版本：v1.0.0
 * 作者：
 * 时间：2025-12-25
 * 版本控制：v1.0.0 - 初始版本，提供基本的RHC输入功能
 * 备注：该对话框用于在临床或研究场景下输入 RHC 检查结果的各项血流动力学参数，
 *      提供清除与保存功能，并支持自定义键盘输入，适配不同分辨率
 ******************************************************************************/

#ifndef RHCINPUTDIALOG_H
#define RHCINPUTDIALOG_H

//1) Project Headers
#include "customkeyboard.h"
#include "closeonlywindow.h"

//2) Qt Headers
#include <QDialog>
#include <QLineEdit>
#include <QEvent>

/**
 * @class RHCInputDialog
 * @brief 右心导管检查数据输入对话框
 *
 * 该对话框用于在临床或研究场景下输入 RHC 检查结果的各项血流动力学参数，
 * 提供清除与保存功能，并支持自定义键盘输入，适配不同分辨率。
 *
 * @note 类中使用了自定义的数字键盘（CustomKeyboard）来辅助输入，并且
 *       支持在不同屏幕分辨率下进行界面缩放（通过scaleX/scaleY来调整界面尺寸）。
 *
 * @param pParent 父窗口，默认为nullptr
 *
 * @see CloseOnlyWindow
 * @see CustomKeyboard
 */

class RHCInputDialog : public CloseOnlyWindow
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数，初始化RHC输入对话框。
     *
     * 该构造函数设置界面布局，初始化自定义键盘，并
     * 根据屏幕分辨率进行界面缩放。
     *
     * @param pParent 父窗口，默认为nullptr。
     */
    explicit RHCInputDialog(QWidget *pParent = nullptr);

    /**
     * @brief 析构函数，释放资源。
     *
     * 该析构函数用于释放在对话框中创建的资源，确保资源的正确管理。
     */
    ~RHCInputDialog();

    /**
     * @brief 获取用户输入的心输出量值。
     *
     * 该方法返回用户在输入框中输入的RA、RV、PA、PCWP 等多组参数。
     *
     * @return 用户输入的心输出量值，类型为QString。
     */
    QString getRHCValue() const;

private:
    QLineEdit *m_pRhcInputEdit; /**< 用于输入RHC数据的输入框 */
    QLineEdit *m_pRaInput1Edit; /**< RA输入框 */
    QLineEdit *m_pRvInput1Edit; /**< RV输入框 */
    QLineEdit *m_pRvInput2Edit; /**< RV输入框 */
    QLineEdit *m_pPaInput1Edit; /**< PA输入框 */
    QLineEdit *m_pPaInput2Edit; /**< PA输入框 */
    QLineEdit *m_pPaInput3Edit; /**< PA输入框 */
    QLineEdit *m_pPcwpInput1Edit; /**< PCWP输入框 */

    QPushButton *m_pClearBtn; /**< 清除按钮 */
    QPushButton *m_pSaveBtn; /**< 保存按钮 */

    CustomKeyboard *m_pCurrentKeyboard = nullptr; /**< 当前自定义数字键盘实例 */

    float m_fScaleX; /**< 屏幕X轴缩放比例 */
    float m_fScaleY; /**< 屏幕Y轴缩放比例 */

    void changeEvent(QEvent *pEvent) override; /**< 处理语言切换事件 */

private slots:
    /**
     * @brief 清除所有输入框内容
     */
    void clearAllFields();
};

#endif // RHCINPUTDIALOG_H
