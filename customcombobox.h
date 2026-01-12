
/********************************************************************************/
/* 文件名    : CustomComboBox.h                                                 */
/* 功能      : 自定义下拉框控件（支持自定义下拉项高度）                         */
/* 版本      : 1.0.0                                                            */
/* 作者      :                                                                  */
/* 日期      : 2025-12-29                                                       */
/* 说明      : 基于 QComboBox 扩展，用于统一 UI 风格并控制下拉项显示高度         */
/********************************************************************************/

#ifndef CUSTOMCOMBOBOX_H
#define CUSTOMCOMBOBOX_H

// 1) Qt Headers
#include <QComboBox>

class CustomComboBox : public QComboBox
{
    Q_OBJECT

public:
    // 构造函数接受选项高度参数
    explicit CustomComboBox(int iTemHeight, QWidget *parent = nullptr);

protected:
    // 重写 pop-up 显示的下拉区域
    void showPopup() override;

private:
    int m_iTemHeight;  // 存储每项的高度
};

#endif // CUSTOMCOMBOBOX_H
