
#ifndef CARDIACOUTPUTDIALOG_H
#define CARDIACOUTPUTDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include "Customkeyboard.h"
#include "CloseOnlyWindow.h"

/**
 * @class CardiacOutputDialog
 * @brief 心输出量输入对话框类
 *
 * 该类继承自 CloseOnlyWindow，用于在血压和平均值已知的情况下，
 * 提供心输出量（Cardiac Output, CO）的输入和显示界面。
 *
 * 功能说明：
 * - 弹出窗口，显示并允许用户输入或修改心输出量。
 * - 提供自定义数字键盘（CustomKeyboard）以方便输入。
 * - 支持根据屏幕分辨率进行界面缩放（scaleX/scaleY）。
 *
 * 成员变量：
 * - QLineEdit *coEdit：用于输入和显示 CO 值的编辑框。
 * - CustomKeyboard *currentKeyboard：当前绑定的自定义键盘对象。
 * - float scaleX, scaleY：界面缩放比例，保证在不同分辨率下界面显示正常。
 *
 * 公共接口：
 * - CardiacOutputDialog(QString bpValue, QString avgValue, QWidget *parent = nullptr)：
 *   构造函数，传入血压值和平均值，初始化对话框。
 * - ~CardiacOutputDialog()：析构函数。
 * - QString getCOValue() const：获取用户输入的心输出量值。
 */

class CardiacOutputDialog : public CloseOnlyWindow {
    Q_OBJECT

public:
    explicit CardiacOutputDialog(QString bpValue, QString avgValue, QWidget *parent = nullptr);
    ~CardiacOutputDialog();
    QString getCOValue() const;

private:
    QLineEdit *coEdit;
    CustomKeyboard* currentKeyboard = nullptr;

    float scaleX;
    float scaleY;
};

#endif // CARDIACOUTPUTDIALOG_H
