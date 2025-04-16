#ifndef PATIENTLISTWIDGET_H
#define PATIENTLISTWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include "customkeyboard.h"

class PatientListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PatientListWidget(QWidget *parent = nullptr);
    ~PatientListWidget();

signals:
    void openSettingsWindow();
    void patientReturnLogin();

protected:
    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    QTableWidget *tableWidget;
    QPushButton *btnBack;
    QPushButton *btnAdd;
    QLineEdit *searchEdit;
    QPushButton *btnSearch;
    CustomKeyboard* currentKeyboard = nullptr;
    bool eventFilterInstalled = false;  // 用来跟踪事件过滤器的安装状态
};

#endif // PATIENTLISTWIDGET_H
