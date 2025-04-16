#ifndef CALIBRATIONDIALOG_H
#define CALIBRATIONDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>

class CalibrationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CalibrationDialog(QWidget* parent = nullptr);
    QString getCalibrationValue() const;

signals:
    void openmonitorwidget();

private:
    QLineEdit* inputEdit;
};

#endif // CALIBRATIONDIALOG_H
