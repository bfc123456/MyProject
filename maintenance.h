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

private slots:
    void closeWidget();  // 关闭窗口

private:
    QLabel *titleLabel;
    QPushButton *closeButton;
};

#endif // MAINTENANCEWIDGET_H
