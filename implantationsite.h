
#ifndef IMPLANTATIONSITE_H
#define IMPLANTATIONSITE_H

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QFrame>
#include <QVBoxLayout>
#include "modernwaveplot.h"
#include "settingswidget.h"
#include "circularprogressbar.h"
#include "CustomMessageBox.h"
#include "calibrationdialog.h"
#include "implantmonitor.h"
#include "FramelessWindow.h"

class ImplantationSite : public FramelessWindow
{
    Q_OBJECT

public:
    explicit ImplantationSite(QWidget *parent = nullptr ,  const QString &sensorId = QString());
     ~ImplantationSite();

signals:
    void returnRequested();

private slots:
    void OpenSettingsRequested();
    
    void startSimulation();
    void updatePlot();   // 定时器更新绘图
    void updateData();   // 更新数据

    void onBtnLocationClicked();
    bool uploadLocation(const QString &loc);

private:
//    LoginWindow *loginWindow;
    QVBoxLayout* mainLayout;
    SettingsWidget *settingswidget;
    QLabel *titleLabel;
    QLabel* implantTitle;
    QLabel* labelCurve;
    QPushButton* helpButton;
    QLabel* heartTitle;
    QLabel* heartUnit;
    QLabel* pressureTitle;
    QLabel* signalTitle;
    QPushButton* returnButton;
    QPushButton* calibrateButton;

    ModernWavePlot* plot;//模拟函数变量声明
    QwtPlotCurve* curve;
    QwtPlotGrid* grid;
    QTimer* timer;
    QVector<double> data;
    QHBoxLayout* secondRow;
    CircularProgressBar* progress;
    QPushButton* buttonL;
    QPushButton* buttonR;
    QString m_serial;
    QLabel *idLabel;
    CalibrationDialog *calibrationialog = nullptr;
    ImplantMonitor *implantmonitor = nullptr;
    bool isClicked = false;  //按钮状态
    QString selectedPos;
    void changeEvent(QEvent *event) override;
};

#endif // IMPLANTATIONSITE_H
