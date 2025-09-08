#ifndef CUSTOMCOMBOBOX_H
#define CUSTOMCOMBOBOX_H

#include <QComboBox>

/**
 * @class CustomComboBox
 * @brief 自定义下拉选择框
 *
 * 本类在 Qt 标准 QComboBox 的基础上进行扩展和封装，
 * 主要用于登录界面、配置界面等场景下的用户选项选择。
 *
 * 功能特点：
 * - 可自定义外观样式（字体、边框、背景色）
 * - 支持虚拟键盘联动（触屏环境下输入更友好）
 * - 可预置常用选项，如“家用模式”“植入模式”
 * - 提供信号与槽机制，方便和其他自定义控件联动
 *
 * 使用场景：
 * - 在 MultiUserLoginWindow 中用作角色选择下拉框
 * - 在设置界面中用作参数配置选择器
 *
 * @note 相比原生 QComboBox，更适合医疗类 UI 的交互风格，
 *       界面简洁且便于触屏点击。
 */


class CustomComboBox : public QComboBox
{
    Q_OBJECT

public:
    // 构造函数接受选项高度参数
    explicit CustomComboBox(int itemHeight, QWidget *parent = nullptr);

protected:
    // 重写 pop-up 显示的下拉区域
    void showPopup() override;

private:
    int m_itemHeight;  // 存储每项的高度
};

#endif // CUSTOMCOMBOBOX_H
