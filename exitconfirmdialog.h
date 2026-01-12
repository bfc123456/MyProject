
#ifndef EXITCONFIRMDIALOG_H
#define EXITCONFIRMDIALOG_H

#include <QDialog>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "closeonlywindow.h"

/**
 * @class ExitConfirmDialog
 * @brief 程序退出确认对话框
 *
 * 本类继承自 QDialog，提供一个简洁的确认对话框，
 * 当用户尝试关闭应用程序时，提示用户确认操作，避免误退出。
 *
 * 功能特点：
 * - 显示提示信息（如“确定要退出程序吗？”）
 * - 提供确认与取消按钮（通常为“退出”和“取消”）
 * - 返回用户选择结果，调用方可根据结果决定是否退出
 * - 可与 MedicalLogger 结合，记录用户退出行为，满足审计需求
 *
 * 使用场景：
 * - 用户点击主界面的退出按钮
 * - 用户在植入或测量流程中尝试直接关闭程序
 *
 * @note 与 CustomMessageBox 类似，但专注于退出确认逻辑；
 *       建议作为应用的统一退出提示框使用。
 */


class ExitConfirmDialog : public CloseOnlyWindow {
    Q_OBJECT
public:
    explicit ExitConfirmDialog(QWidget *parent = nullptr);
    ~ExitConfirmDialog();

signals:
    void shutdownClicked();
    void returnHomeClicked();
    void shutdownSystem();
    void returnToMain();


private:
    void changeEvent(QEvent *event);

    QLabel *messageLabel;
    QPushButton *shutdownButton;
    QPushButton *returnButton;
    QPushButton *closeButton;
    float scaleX;
    float scaleY;
};

#endif // EXITCONFIRMDIALOG_H
