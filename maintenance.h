#ifndef MAINTENANCEWIDGET_H
#define MAINTENANCEWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>

class MaintenanceWidget : public QWidget {
    Q_OBJECT

public:
    explicit MaintenanceWidget(QWidget *parent = nullptr);
    ~MaintenanceWidget();

private:
    QLabel *titleLabel;
};

#endif // MAINTENANCEWIDGET_H
