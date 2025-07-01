
#ifndef CARDIACOUTPUTDIALOG_H
#define CARDIACOUTPUTDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include "customkeyboard.h"
#include "CloseOnlyWindow.h"

class CardiacOutputDialog : public CloseOnlyWindow {
    Q_OBJECT

public:
    explicit CardiacOutputDialog(QString bpValue, QString avgValue, QWidget *parent = nullptr);
    ~CardiacOutputDialog();
    QString getCOValue() const;

private:
    QLineEdit *coEdit;
    CustomKeyboard* currentKeyboard = nullptr;

    float scaleX;
    float scaleY;
};

#endif // CARDIACOUTPUTDIALOG_H
