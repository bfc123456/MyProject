
#ifndef RHCINPUTDIALOG_H
#define RHCINPUTDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include "Customkeyboard.h"
#include "CloseOnlyWindow.h"
#include <QEvent>

/**
 * @class RHCInputDialog
 * @brief 右心导管检查（Right Heart Catheterization, RHC）数据输入对话框
 *
 * 该对话框用于在临床或研究场景下输入 RHC 检查结果的各项血流动力学参数，
 * 提供清除与保存功能，并支持自定义键盘输入，适配不同分辨率。
 *
 * 功能：
 * - 输入 RHC 相关数值：RA、RV、PA、PCWP 等多组参数。
 * - clearAllFields()：一键清空输入框，便于重新录入。
 * - getRHCValue()：获取汇总的 RHC 输入结果（如 JSON/字符串形式）。
 * - 提供保存按钮，用于确认并传递数据。
 * - 支持多语言切换（重写 changeEvent()）。
 *
 * 输入字段：
 * - rhcEdit：RHC 总值/标识
 * - raInput1：右心房（RA）数值
 * - rvInput1, rvInput2：右心室（RV）收缩压/舒张压
 * - paInput1, paInput2, paInput3：肺动脉（PA）相关数值
 * - pcwpInput1：肺毛细血管楔压（PCWP）
 *
 * 操作控件：
 * - clearButton：清空所有输入
 * - saveButton：保存/确认输入
 * - currentKeyboard：绑定的自定义软键盘
 *
 * 内部特性：
 * - scaleX/scaleY：界面缩放比例，适配高 DPI/不同分辨率。
 */

class RHCInputDialog : public CloseOnlyWindow
{
    Q_OBJECT

public:
    explicit RHCInputDialog(QWidget *parent = nullptr);
    ~RHCInputDialog();
    QString getRHCValue() const;

private: 
    QLineEdit *rhcEdit;
    QLineEdit *raInput1;

    QLineEdit *rvInput1;
    QLineEdit *rvInput2;
    QLineEdit *paInput1;
    QLineEdit *paInput2;
    QLineEdit *paInput3;

    QLineEdit *pcwpInput1;
    QPushButton *clearButton;
    QPushButton *saveButton;
    CustomKeyboard* currentKeyboard = nullptr;
    void changeEvent(QEvent *event) override;

    float scaleX;
    float scaleY;

private slots:
    void clearAllFields();
};

#endif // RHCINPUTDIALOG_H
