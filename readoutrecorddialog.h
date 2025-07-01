
#include <QDialog>
#include <QTableWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include "measurementdata.h"

class ReadoutRecordDialog : public QDialog {
    Q_OBJECT

public:
    explicit ReadoutRecordDialog(QWidget *parent = nullptr);
    ~ ReadoutRecordDialog();

public slots:
    void populateData(const QList<MeasurementData> &list);

private slots:
    void onDeleteButtonClicked();//删除动作函数

signals:
    void rowDeleted(int row);
    void returnImplantationsite();
    void onRefreshButtonClicked();

private:
    QTableWidget *table;
    QPushButton *btnRefresh;
    QPushButton *btnDelete;
    float scaleX;
    float scaleY;
};
