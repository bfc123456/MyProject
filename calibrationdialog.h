
#ifndef CALIBRATIONDIALOG_H
#define CALIBRATIONDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <Customkeyboard.h>
#include <QCloseEvent>
#include "CloseOnlyWindow.h"

/**
 * @class CalibrationDialog
 * @brief 设备校准对话框
 *
 * 本对话框用于设备的校准操作，确保测量数据的准确性与可靠性。
 * 医生或维护人员可以通过该界面对传感器或测量模块进行校准，
 * 并将校准结果记录到系统中以保证可追溯性。
 *
 * 主要功能包括：
 * - 提供交互界面指导用户完成设备校准步骤
 * - 支持输入、保存和验证校准参数
 * - 将校准结果写入数据库，满足医疗系统追溯要求
 * - 在日志中记录校准操作，便于后续审计
 *
 * @note 本类通常由医生操作流程触发，
 *       是保障设备长期使用准确度的重要环节。
 */


class CalibrationDialog : public CloseOnlyWindow
{
    Q_OBJECT

public:
    explicit CalibrationDialog(QWidget* parent = nullptr);
    QString getCalibrationValue() const;

signals:
    void openmonitorwidget();

private:
    QLineEdit* inputEdit;
    CustomKeyboard *currentKeyboard;
    bool eventFilterInstalled = false;  // 用来跟踪事件过滤器的安装状态
    void changeEvent(QEvent *event) override;
    QLabel* title;
    QLabel* inputLabel;
    QPushButton* resetBtn;
    QPushButton* saveBtn;

    float scaleX;
    float scaleY;
};

#endif // CALIBRATIONDIALOG_H
