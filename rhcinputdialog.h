#ifndef RHCINPUTDIALOG_H
#define RHCINPUTDIALOG_H

#include <QDialog>
#include <QLineEdit>

class RHCInputDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RHCInputDialog(QWidget *parent = nullptr);
    ~RHCInputDialog();
    QString getRHCValue() const;

private:
    QLineEdit *raEdit;  // RA输入框
    QLineEdit *rvEdit;  // RV输入框
    QLineEdit *paEdit;  // PA输入框
    QLineEdit *pcwpEdit; // PCWP输入框
    QLineEdit *rhcEdit;
    QPushButton *clearButton;
    QPushButton *saveButton;
};

#endif // RHCINPUTDIALOG_H
