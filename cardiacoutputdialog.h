#ifndef CARDIACOUTPUTDIALOG_H
#define CARDIACOUTPUTDIALOG_H

#pragma once

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

class CardiacOutputDialog : public QDialog {
    Q_OBJECT

public:
    explicit CardiacOutputDialog(QString bpValue, QString avgValue, QWidget *parent = nullptr);
    ~CardiacOutputDialog();
    QString getCOValue() const;

private:
    QLineEdit *coEdit;
};

#endif // CARDIACOUTPUTDIALOG_H
