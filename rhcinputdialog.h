
#ifndef RHCINPUTDIALOG_H
#define RHCINPUTDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include "customkeyboard.h"
#include "CloseOnlyWindow.h"
#include <QEvent>

class RHCInputDialog : public CloseOnlyWindow
{
    Q_OBJECT

public:
    explicit RHCInputDialog(QWidget *parent = nullptr);
    ~RHCInputDialog();
    QString getRHCValue() const;

private: 
    QLineEdit *rhcEdit;
    QLineEdit *raInput1;

    QLineEdit *rvInput1;
    QLineEdit *rvInput2;
    QLineEdit *paInput1;
    QLineEdit *paInput2;
    QLineEdit *paInput3;

    QLineEdit *pcwpInput1;
    QPushButton *clearButton;
    QPushButton *saveButton;
    CustomKeyboard* currentKeyboard = nullptr;
    void changeEvent(QEvent *event) override;

    float scaleX;
    float scaleY;

private slots:
    void clearAllFields();
};

#endif // RHCINPUTDIALOG_H
