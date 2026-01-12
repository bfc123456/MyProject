
/********************************************************************************/
/* 文件名    : SettingsWidget.h                                                 */
/* 功能      : 系统设置界面（包括信号强度、语言切换、系统操作等）               */
/* 版本      : 1.0.0                                                              */
/* 作者      :                                                       */
/* 日期      : 2025-12-26                                                       */
/* 说明      : 该文件定义了系统设置界面，提供信号强度调整、语言切换及其他系统操作功能   */
/********************************************************************************/

#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

//1) Project Headers
#include "framelesswindow.h"

//2) Qt Headers
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QComboBox>

//前置对象声明
class LoginWindow;

/**
 * @class SettingsWidget
 * @brief 系统设置界面
 *
 * 本类继承自 FramelessWindow，提供一个无边框的系统设置界面。
 * 用户可以在该界面中调整信号强度、切换语言，以及执行系统级操作（如重启和关机）。
 *
 * 功能职责：
 * - 显示并修改信号强度（通过滑块控件）
 * - 提供语言切换下拉框，实现多语言切换
 * - 提供重启和关机按钮，触发相应的操作
 * - 显示软件版本与系统信息
 * - 通过信号通知外部界面该窗口被关闭或设置发生变化
 *
 * 使用场景：
 * - 用户在主界面或维护模式下进入系统设置
 * - 医疗设备的参数调整与多语言支持
 *
 * @note
 * - 信号 `signalStrengthChanged(int)` 用于通知外部信号强度参数更新
 * - 建议与 LanguageManager、MedicalLogger 联动，记录语言切换和系统操作日志
 */

class SettingsWidget : public FramelessWindow
{
    Q_OBJECT

public:
    explicit SettingsWidget(QWidget *parent = nullptr);
    ~SettingsWidget();

signals:
    void SigRequestDelete(SettingsWidget* self);   // 请求销毁自身
    void SigSettingsWidgetClose();
    void SigStrengthChanged(int value);  // 信号强度变化时发出

protected:
    void changeEvent(QEvent *event) override;   // 响应语言切换事件

private slots:
    void SlotOInLanguageChanged(const QString &);    // 语言切换处理槽函数
    void SlotOnRebootClicked();                     // 重启按钮点击
    void SlotOnShutdownClicked();                   // 关机按钮点击
    void SlotOnBtnCloseClicked();                   // 关闭按钮点击
    void SlotOnUpdateClicked();                     // 软件更新按钮点击

private:
    // UI 控件
    QLabel* m_pSystemSettingsLabel {nullptr};
    QLabel* m_pSignalStrengthLabel {nullptr};
    QLabel* m_pLanguageLabel {nullptr};
    QLabel* m_pRebootLabel {nullptr};
    QLabel* m_pShutdownLabel {nullptr};
    QLabel* m_pSystemInfoLabel {nullptr};
    QLabel* m_pUpdateLabel {nullptr};
    QLabel* m_pSoftwareVersionLabel1 {nullptr};
    QLabel* m_pSoftwareVersionLabel2 {nullptr};
    QLabel* m_pTitleLabel {nullptr};

    // 控件
    QSlider* m_pSignalStrengthSlider {nullptr};
    QComboBox* m_pLanguageComboBox {nullptr};
    QPushButton* m_pModifyButton {nullptr};
    QPushButton* m_pRebootButton {nullptr};
    QPushButton* m_pShutdownButton {nullptr};
    QPushButton* m_pUpdateButton {nullptr};

    // 界面缩放
    float m_fScaleX {1.0f};
    float m_fScaleY {1.0f};

    void Retranslate();                    // 统一刷新控件文本
};

#endif // SETTINGSWIDGET_H
