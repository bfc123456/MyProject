
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
#include "custommessagebox.h"
#include "calibrationdialog.h"
#include "implantmonitor.h"
#include "framelesswindow.h"
#include <qwt_plot_grid.h>

/**
 * @class ImplantationSite
 * @brief 植入位置选择与监测界面
 *
 * 本界面用于医生在植入场景下，选择传感器植入位置（左/右），
 * 并实时显示模拟波形、信号强度等信息，同时可进入校准和监测流程。
 *
 * 功能职责：
 * - 显示植入位置选择按钮（左 / 右）
 * - 展示实时波形图、信号强度进度条
 * - 提供“校准”按钮，进入 CalibrationDialog 完成设备校准
 * - 提供“监测”按钮，进入 ImplantMonitor 进行实时数据测量
 * - 通过 returnRequested() 信号返回上一级界面
 *
 * 使用场景：
 * - 植入注册完成后，医生进入该界面选择植入位置
 * - 在正式测量前检查信号质量，必要时进行校准
 *
 * @note 本类继承 FramelessWindow，保持无边框风格。
 */


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
    
    void startSimulation(); //启动波形模拟
    void updatePlot();   // 定时器更新绘图
    void updateData();   // 更新数据

    void onBtnLocationClicked();
    bool uploadLocation(const QString &loc);    //更新植入位置

private:
    QVBoxLayout* mainLayout;
    QHBoxLayout* secondRow;

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
    CircularProgressBar* progress;
    QPushButton* buttonL;
    QPushButton* buttonR;
    QString m_serial;
    QLabel *idLabel;

//    SettingsWidget *settingswidget;
    CalibrationDialog *calibrationialog = nullptr;
    ImplantMonitor *implantmonitor = nullptr;

    bool isClicked = false;  //按钮状态
    QString selectedPos;

    float scaleX;
    float scaleY;

    void changeEvent(QEvent *event) override;
};

#endif // IMPLANTATIONSITE_H
