#pragma once

#include <QDialog>
#include <QTableWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>

class ReadoutRecordDialog : public QDialog {
    Q_OBJECT

public:
    explicit ReadoutRecordDialog(QWidget *parent = nullptr);
    ~ ReadoutRecordDialog();

private:
    QTableWidget *table;
    QPushButton *btnRefresh;
    QPushButton *btnDelete;
    void populateData();
};
