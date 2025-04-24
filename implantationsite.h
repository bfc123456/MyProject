#ifndef IMPLANTATIONSITE_H
#define IMPLANTATIONSITE_H

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QFrame>
#include <qwt_plot.h>
#include <QVBoxLayout>
#include <qwt_plot_grid.h>//模拟函数头文件（后续可删除）
#include <qwt_plot_curve.h>//模拟函数头文件（后续可删除）
//#include "loginwindow.h"
#include "settingswidget.h"
#include "circularprogressbar.h"
#include "CustomMessageBox.h"

class ImplantationSite : public QWidget
{
    Q_OBJECT

public:
    explicit ImplantationSite(QWidget *parent = nullptr ,  const QString &sensorId = QString());
     ~ImplantationSite();

signals:
    void returnRequested();

private slots:
    void OpenSettingsRequested();
//    void openImplantMonitorWidget();
    
    void startSimulation();
    void updatePlot();   // 定时器更新绘图
    void updateData();   // 更新数据

    void onBtnLocationClicked();
    bool uploadLocation(const QString &loc);

private:
//    LoginWindow *loginWindow;
    QVBoxLayout* mainLayout;
    SettingsWidget *settingswidget;

    QwtPlot* plot;//模拟函数变量声明
    QwtPlotCurve* curve;
    QwtPlotGrid* grid;
    QTimer* timer;
    QVector<double> data;
    QHBoxLayout* secondRow;
    CircularProgressBar* progress;
    QPushButton* buttonL;
    QPushButton* buttonR;
    QString m_serial;

};

#endif // IMPLANTATIONSITE_H
