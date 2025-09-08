
#ifndef CUSTOMMESSAGEBOX_H
#define CUSTOMMESSAGEBOX_H

#include <QDialog>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPixmap>
#include "CloseOnlyWindow.h"

/**
 * @class CustomMessageBox
 * @brief 自定义消息对话框
 *
 * 本类在 Qt 标准 QMessageBox 基础上进行了定制化封装，
 * 用于在系统中显示提示、警告或确认信息，并提供灵活的按钮配置。
 *
 * 功能特点：
 * - 支持自定义标题、内容文本和按钮标签
 * - 可通过构造函数指定对话框宽度，适配不同分辨率和触屏设备
 * - 按钮排列和样式可定制，界面风格与应用整体一致
 * - 提供阻塞/非阻塞调用模式，方便业务逻辑使用
 * - 结合 MedicalLogger，可记录用户的选择结果，满足审计和追溯要求
 *
 * 使用场景：
 * - 医疗操作提示（如“请将传感器植入患者体内”）
 * - 确认类操作（如“是否保存测量数据？”）
 * - 警告或错误提示（如“数据库连接失败”）
 *
 * @note 相比标准 QMessageBox，本类更适合医疗场景，
 *       因为其布局和按钮命名可控，能减少误操作风险。
 */


class CustomMessageBox : public CloseOnlyWindow
{
    Q_OBJECT

public:
    explicit CustomMessageBox(QWidget *parent = nullptr, const QString &title = "信息", const QString &message = "",
                              const QVector<QString> &buttons = {}, int width = 300);

    QString getUserResponse();  // 获取用户的响应

private:
    QVector<QPushButton *> buttonList;
    QLabel *iconLabel;
    QLabel *messageLabel;
    QVBoxLayout *mainLayout;
    QHBoxLayout *buttonLayout;
    QString userResponse;
    float scaleX;
    float scaleY;

    const QString defaultIconPath = ":image/exclamation_mark.png";  // 默认图标路径
};

#endif // CUSTOMMESSAGEBOX_H
