
/********************************************************************************/
/* 文件名    : closeonlywindow.h                                                 */
/* 功能      : 消息对话框自定义界面（去除最小化与最大化，仅保留关闭按钮）               */
/* 版本      : 1.0.0                                                              */
/* 作者      :                                                         */
/* 日期      : 2025-12-26                                                       */
/* 说明      : 该文件定义了消息对话框界面，提供界面关闭功能   */
/********************************************************************************/

#ifndef CUSTOMMESSAGEBOX_H
#define CUSTOMMESSAGEBOX_H

//1) Project Headers
#include "closeonlywindow.h"

//2) Qt Headers
#include <QDialog>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPixmap>

//构造函数
class CustomMessageBox : public CloseOnlyWindow
{
    Q_OBJECT

public:
    explicit CustomMessageBox(QWidget *pParent = nullptr, const QString &strTitle = "信息", const QString &strMessage = "",
                              const QVector<QString> &vecStrButtons = {}, int iWidth = 300);

    ~CustomMessageBox();
    QString GetUserResponse();  // 获取用户的响应

private:
    QVector<QPushButton *> vecBtnList = {};                // 初始化为空的按钮列表
    QLabel* m_pIconLabel {nullptr};                         // 初始化为空
    QLabel* m_pMessageLabel {nullptr};                      // 初始化为空
    QHBoxLayout* m_pButtonLayout {nullptr};                 // 初始化为空
    QString strUserResponse = "";                           // 初始化为空字符串
    float m_fScaleX = 1.0f;                                // 默认缩放因子 X 为 1.0
    float m_fScaleY = 1.0f;                               // 默认缩放因子 Y 为 1.0

    const QString strDefaultIconPath = ":/image/exclamation_mark1.png";  // 默认图标路径
};

#endif // CUSTOMMESSAGEBOX_H
