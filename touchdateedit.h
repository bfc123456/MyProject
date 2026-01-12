#ifndef TOUCHDATEEDIT_H
#define TOUCHDATEEDIT_H

#include <QDateEdit>

/**
 * @brief TouchDateEdit
 * 触控友好的日期输入控件：
 * 1) 单击输入框任意位置即可弹出日历（不需要精准点右侧箭头）
 * 2) 支持统一样式：编辑框 + 日历弹窗
 *
 * 说明：
 * - Qt 5.12 下 QDateEdit/QDateTimeEdit 的 showPopup() 在某些环境/头文件可见性会踩坑，
 *   这里采用“模拟点击下拉箭头”的方式打开弹窗，并加重入保护避免递归崩溃。
 */
class TouchDateEdit : public QDateEdit
{
    Q_OBJECT
public:
    /**
     * @brief TouchDateEdit 构造函数
     * @param parent 父控件
     */
    explicit TouchDateEdit(QWidget *parent = nullptr);

    /**
     * @brief ~TouchDateEdit 析构函数
     */
    ~TouchDateEdit() override;

    /**
     * @brief applyTouchStyle 应用触控风格（编辑框 + 日历弹窗）
     * @param scaleX 横向缩放
     * @param scaleY 纵向缩放
     * @param popupWidth 日历弹窗宽（逻辑值，会乘缩放）
     * @param popupHeight 日历弹窗高（逻辑值，会乘缩放）
     * @param editFontPt 编辑框字体大小（pt）
     * @param calFontPt 日历日期字体大小（pt）
     * @return void
     */
    void applyTouchStyle(float scaleX, float scaleY,
                         int popupWidth = 720, int popupHeight = 520,
                         int editFontPt = 16, int calFontPt = 14);

    /**
     * @brief setOpenOnClick 是否允许单击任意位置弹出日历
     * @param enable true=开启；false=关闭
     * @return void
     */
    void setOpenOnClick(bool enable);

protected:
    /**
     * @brief eventFilter 事件过滤器：捕获点击并打开日历
     * @param watched 被监控对象
     * @param event 事件
     * @return true 表示事件被消耗；false 继续传递
     */
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    bool m_openOnClick = true;     ///< 是否开启“单击打开日历”
    bool m_inSynthClick = false;   ///< 重入保护：避免模拟点击再触发 eventFilter 递归
};

#endif // TOUCHDATEEDIT_H
