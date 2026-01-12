
#ifndef CLOSEONLYWINDOW_H
#define CLOSEONLYWINDOW_H

#include <QDialog>

/**
 * @class CloseOnlyWindow
 * @brief 仅带关闭按钮的对话框
 *
 * 本类继承自 QDialog，封装了一个简化的对话框窗口，
 * 去除了最大化和最小化按钮，仅保留标题栏和关闭按钮。
 *
 * 使用场景：
 * - 用于显示提示信息或确认信息，不需要用户调整窗口大小
 * - 常见于“只读提示”或“系统消息”类弹窗
 *
 * 功能特点：
 * - 窗口风格简洁，避免用户误点最小化/最大化
 * - 保持与 Qt 标准 QDialog 的兼容性
 *
 * @note 如果需要添加更多功能（如按钮、输入框），
 *       可以在该类基础上扩展。
 */


class CloseOnlyWindow : public QDialog {
    Q_OBJECT
public:
    explicit CloseOnlyWindow(QWidget *parent = nullptr)
      : QDialog(parent)
    {
        // 去掉标题栏
        setWindowFlags( (Qt::Dialog | Qt::FramelessWindowHint)
                        & ~Qt::WindowSystemMenuHint
                        & ~Qt::WindowMinMaxButtonsHint
                        & ~Qt::WindowCloseButtonHint
                        );
    }
};

#endif // CLOSEONLYWINDOW_H
