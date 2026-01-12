#ifndef DEBUGMODESELECTOR_H
#define DEBUGMODESELECTOR_H

#include<QDialog>
#include "closeonlywindow.h"

/**
 * @class DebugModeSelector
 * @brief 调试模式选择器
 *
 * 本类提供一个调试模式选择界面，允许用户在不同调试方式之间进行切换，
 * 并通过信号机制将用户选择传递给外部逻辑。
 *
 * 功能特点：
 * - 提供界面交互，用户可选择调试模式（如 "serial" 串口模式、"udp" 网络调试模式）
 * - 通过信号 modeSelected(QString) 将选择结果通知外部
 * - 与 SerialDebugWidget、udpDebugWidget 等调试工具窗口联动
 *
 * 使用场景：
 * - 维护界面中，用户点击触发调试模式选择
 * - 工程人员在开发/测试阶段，通过选择模式快速进入不同调试工具
 *
 * @note 本类自身只负责调试模式选择逻辑，不直接实现调试功能，
 *       调试功能由外部具体的 Widget（如 udpDebugWidget）负责。
 */


class DebugModeSelector : public CloseOnlyWindow {
    Q_OBJECT

public:
    explicit    DebugModeSelector(QWidget *parent = nullptr);    //防止隐式类型转换

signals:
    void modeSelected(QString mode);

private:
    float scaleX;
    float scaleY;
};

#endif // DEBUGMODESELECTOR_H
