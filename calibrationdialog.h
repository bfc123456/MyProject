#ifndef CALIBRATIONDIALOG_H
#define CALIBRATIONDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <customkeyboard.h>
#include <QCloseEvent>

class CalibrationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CalibrationDialog(QWidget* parent = nullptr);
    QString getCalibrationValue() const;

signals:
    void openmonitorwidget();


//protected:
//    void closeEvent(QCloseEvent *event) override;
//    void showEvent(QShowEvent *event) override;

private:
    QLineEdit* inputEdit;
    CustomKeyboard *currentKeyboard;
    bool eventFilterInstalled = false;  // 用来跟踪事件过滤器的安装状态
};

#endif // CALIBRATIONDIALOG_H
