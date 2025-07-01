
#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QComboBox>
#include "FramelessWindow.h"

class LoginWindow;

class SettingsWidget : public FramelessWindow
{
    Q_OBJECT

public:
    explicit SettingsWidget(QWidget *parent = nullptr);
    ~SettingsWidget();

signals:
    void requestDelete(SettingsWidget* self);
    void settingsWidgetClose();

protected:
    void changeEvent(QEvent *event) override;

private slots:
    void onLanguageChanged(const QString &);
    void onRebootClicked();
    void onShutdownClicked();
    void onBtnCloseClicked();

private:
    QLabel *systemSettingsLabel;
    QLabel *signalStrengthLabel;
    QLabel *languageLabel;
    QLabel *rebootLabel;
    QLabel *shutdownLabel;
    QLabel *systemInfoLabel;
    QLabel *softwareVersionLabel1;
    QLabel *softwareVersionLabel2;
    QLabel *titleLabel;

    QSlider *signalStrengthSlider;
    QComboBox *languageComboBox;
    QPushButton *modifyButton;
    QPushButton *rebootButton;
    QPushButton *shutdownButton;
};

#endif // SETTINGSWIDGET_H
