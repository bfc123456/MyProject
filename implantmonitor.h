
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
#include "modernwaveplot.h"
#include "readoutrecorddialog.h"
#include "measurewidget.h"
#include "reviewwidget.h"
#include "FramelessWindow.h"

class ImplantMonitor : public FramelessWindow {
    Q_OBJECT
public:
    explicit ImplantMonitor(QWidget *parent = nullptr ,  const QString &sensorId = QString());
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

    QwtPlotCurve *curve;
    QLineEdit *rhcEdit;
    QList<MeasurementData> measurementList;
    SettingsWidget *settingswidget;
    ReadoutRecordDialog *readoutdialog = nullptr;
    MeasureWidget *measurewidget;
    bool readOutFlg = false;
    ModernWavePlot *plot;
    void createTempDatabase();
    ReviewWidget *reviewwidget = nullptr;
    QString m_serial;
    bool m_isLeft  = true;  // 默认

signals:
    void dataListUpdated(const QList<MeasurementData>& list);   //表格更新提示
    void returnImplantationsite();

private slots:
    void openCOClicked();
    void openRHCClicked();
    void openMeasureClicked();
    void openReviewClicked();
    //    void openReadoutClicked();
    void OpenSettingsRequested();
    void onDataSaved(const MeasurementData &d);
    void onReadoutButtonClicked();
//    void onDataListUpdated(const QList<MeasurementData>& list);  //更新结构体
    void onRowDeleted(int row); //删除结构体特定行
    void changeEvent(QEvent *event) override;
};


#endif // IMPLANTMONITOR_H
