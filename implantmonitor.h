#ifndef IMPLANTMONITOR_H
#define IMPLANTMONITOR_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include "cardiacoutputdialog.h"
#include "settingswidget.h"

class ImplantMonitor : public QWidget {
    Q_OBJECT
public:
    explicit ImplantMonitor(QWidget *parent = nullptr);
    ~ImplantMonitor();
private:
    QLabel *titleLabel;
    QLabel *idLabel;
    QLabel *bpLabel;
    QLabel *hrLabel;
    QLabel *imgLabel;
    QLabel *bpVal;
    QLabel *avgVal;
    QLabel *hrVal;

    QProgressBar *progress;
    QPushButton *startBtn;
    QPushButton *inputCO;
    QPushButton *inputRHC;
    QPushButton *statBtn;
    QPushButton *statisticsbtn;

    QwtPlot *plot;
    QwtPlotCurve *curve;
    QLineEdit *rhcEdit;
    SettingsWidget *settingswidget;


private slots:
    void openCOClicked();
    void openRHCClicked();
    void openMeasureClicked();
    void openReviewClicked();
    void openReadoutClicked();
    void OpenSettingsRequested();
};


#endif // IMPLANTMONITOR_H
