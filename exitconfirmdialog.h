
#ifndef EXITCONFIRMDIALOG_H
#define EXITCONFIRMDIALOG_H

#include <QDialog>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "CloseOnlyWindow.h"

class ExitConfirmDialog : public CloseOnlyWindow {
    Q_OBJECT
public:
    explicit ExitConfirmDialog(QWidget *parent = nullptr);
    ~ExitConfirmDialog();

signals:
    void shutdownClicked();
    void returnHomeClicked();

private:
    QLabel *messageLabel;
    QPushButton *shutdownButton;
    QPushButton *returnButton;
    QPushButton *closeButton;
    float scaleX;
    float scaleY;
};

#endif // EXITCONFIRMDIALOG_H
