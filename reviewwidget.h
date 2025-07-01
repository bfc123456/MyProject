
#ifndef REVIEWWIDGET_H
#define REVIEWWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QTableWidget>
#include <QGroupBox>
#include <QPixmap>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "settingswidget.h"
#include "measurementdata.h"
#include "modernwaveplot.h"
#include "databasemanager.h"
#include "FramelessWindow.h"

class ReviewWidget : public FramelessWindow{
    Q_OBJECT

public:
    explicit ReviewWidget(QWidget *parent = nullptr ,  const QString &sensorId = QString());
    ~ReviewWidget();
    void setDataList(const QList<MeasurementData> &list);

signals:
    void returnToMainWindowSignal();
    void returnToImplantmonitor();

private slots:
    void showExitConfirmWidget();
    void OpenSettingsRequested();
    bool uploadToDatabase();

private:
    SettingsWidget *settingswidget;
    QTableWidget *historyTable  = nullptr;
    QPushButton *saveButton;
    QLabel *titleLabel;
    QLabel *monitorpressure;
    QLabel *hrLabel;
    QLabel *historylabel;
    QPushButton *backButton;
    QString m_serial;
    QLabel *idLabel;
    ModernWavePlot *bpPlot  = nullptr;
    QList<MeasurementData> m_list;
    void changeEvent(QEvent *event) override;

    float scaleX;
    float scaleY;
};

#endif // REVIEWWIDGET_H
