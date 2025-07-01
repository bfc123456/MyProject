#ifndef CUSTOMCOMBOBOX_H
#define CUSTOMCOMBOBOX_H

#include <QComboBox>

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
