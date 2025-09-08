
#include <QDateEdit>
#include <QCalendarWidget>
#include <QEvent>

/**
 * @class CustomDateEdit
 * @brief 自定义日期输入控件
 *
 * 本类继承自 Qt 的 QDateEdit，针对医疗/业务场景做了定制化封装，
 * 用于在界面中录入和选择日期（如患者出生日期、植入日期、复查日期等）。
 *
 * 功能特点：
 * - 保留日历弹出选择功能，用户可通过点击选择日期
 * - 支持格式化显示（如 yyyy-MM-dd），统一数据录入规范
 * - 可与数据库或表单系统直接绑定，避免输入错误
 * - 支持校验日期范围（如禁止选择未来日期或过早的日期）
 * - 可定制样式，适配触屏和桌面双场景
 *
 * 使用场景：
 * - 患者信息绑定界面中填写出生日期
 * - 植入登记界面中填写手术日期
 * - 设置或统计界面中选择时间范围
 *
 * @note 相比原生 QDateEdit，本控件更注重输入约束和医疗软件的合规性，
 *       确保日期数据的合法性和可追溯性。
 */


class CustomDateEdit : public QDateEdit {
    Q_OBJECT
public:
    explicit CustomDateEdit(QWidget *parent = nullptr);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
};
