#ifndef MEASUREWIDGET_H
#define MEASUREWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_symbol.h>
#include "settingswidget.h"

class MeasureWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MeasureWidget(QWidget *parent = nullptr);
    ~MeasureWidget();

signals:
    void returnprepage();

private:
    QwtPlot *plot;  // 用于绘制曲线的QwtPlot组件
    QLabel *titleLabel;
    QLabel *idLabel;
    QLineEdit *systolicPressureEdit;
    QLineEdit *diastolicPressureEdit;
    QLineEdit *avgPressureEdit;
    QLineEdit *heartRateEdit;
    QLineEdit *coEdit;

    // 按钮
    QPushButton *saveButton;
    QPushButton *clearButton;
    SettingsWidget *settingswidget;

private slots:
    void OpenSettingsRequested();
    void returnImplantmonitor();
    void openSaveConfirm();
};

#endif // MEASUREWIDGET_H
