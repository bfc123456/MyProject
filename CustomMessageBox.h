#ifndef CUSTOMMESSAGEBOX_H
#define CUSTOMMESSAGEBOX_H

#include <QDialog>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPixmap>

class CustomMessageBox : public QDialog
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

    const QString defaultIconPath = ":image/exclamation_mark.png";  // 默认图标路径
};

#endif // CUSTOMMESSAGEBOX_H
