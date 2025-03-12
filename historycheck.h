#ifndef HISTORYCHECK_H
#define HISTORYCHECK_H

#include <QMainWindow>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include "ui_historycheck.h"
#include "databasemanager.h"
#include "customkeyboard.h"

namespace Ui {
class HistoryCheck;
}

class HistoryCheck : public QMainWindow
{
    Q_OBJECT

public:
    explicit HistoryCheck(databasemanager *db,QWidget *parent = nullptr);
    ~HistoryCheck();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

//新添加代码
private slots:
     void onKeyboardKeyPressed(const QString &text);

private:
    Ui::HistoryCheck *ui;
    QTableWidget *historyTable; // 历史数据表格
    void loadHistoryData(); // 加载历史数据函数
    void searchData();
    void deleteData();

    QPushButton *deleteButton;  // 删除按钮
    QPushButton *searchButton;
    databasemanager *dbManager;
    QLineEdit *searchLineEdit;
    CustomKeyboard *customKeyboard;//新添加代码

};

#endif // HISTORYCHECK_H
