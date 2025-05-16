
#ifndef CALIBRATIONDIALOG_H
#define CALIBRATIONDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <customkeyboard.h>
#include <QCloseEvent>
#include "CloseOnlyWindow.h"

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
};

#endif // CALIBRATIONDIALOG_H
