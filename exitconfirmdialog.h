#ifndef EXITCONFIRMDIALOG_H
#define EXITCONFIRMDIALOG_H

#include <QDialog>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

class ExitConfirmDialog : public QDialog {
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
};

#endif // EXITCONFIRMDIALOG_H
