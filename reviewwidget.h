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
#include <qwt_plot.h>
#include <qwt_plot_curve.h>

class ReviewWidget : public QWidget {
    Q_OBJECT

public:
    explicit ReviewWidget(QWidget *parent = nullptr);
    ~ReviewWidget();

signals:
    void returnToMainWindowSignal();
    void returnToImplantmonitor();

private slots:
    void showExitConfirmWidget();
    void OpenSettingsRequested();

private:
    SettingsWidget *settingswidget;
};

#endif // REVIEWWIDGET_H
