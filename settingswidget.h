
#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QComboBox>
#include "FramelessWindow.h"


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
    void requestDelete(SettingsWidget* self);   // 请求销毁自身
    void settingsWidgetClose();
    void signalStrengthChanged(int value);  // 信号强度变化时发出

protected:
    void changeEvent(QEvent *event) override;   // 响应语言切换事件

private slots:
    void onLanguageChanged(const QString &);    // 语言切换处理槽函数
    void onRebootClicked();                     // 重启按钮点击
    void onShutdownClicked();                   // 关机按钮点击
    void onBtnCloseClicked();                   // 关闭按钮点击
    void onUpdateClicked();                     // 软件更新按钮点击

private:
    QLabel *systemSettingsLabel;
    QLabel *signalStrengthLabel;
    QLabel *languageLabel;
    QLabel *rebootLabel;
    QLabel *shutdownLabel;
    QLabel *systemInfoLabel;
    QLabel *updateLabel;
    QLabel *softwareVersionLabel1;
    QLabel *softwareVersionLabel2;
    QLabel *titleLabel;

    QSlider *signalStrengthSlider;
    QComboBox *languageComboBox;
    QPushButton *modifyButton;
    QPushButton *rebootButton;
    QPushButton *shutdownButton;
    QPushButton *updateButton;

    float scaleX;
    float scaleY;
};

#endif // SETTINGSWIDGET_H
