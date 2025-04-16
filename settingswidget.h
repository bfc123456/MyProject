#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QComboBox>

class LoginWindow;

class SettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsWidget(QWidget *parent = nullptr);
    ~SettingsWidget();

protected:
    void changeEvent(QEvent *event) override;

private slots:
    void onLanguageChanged(const QString &);
    void onRebootClicked();
    void onShutdownClicked();

private:
    QLabel *systemSettingsLabel;
    QLabel *signalStrengthLabel;
    QLabel *languageLabel;
    QLabel *rebootLabel;
    QLabel *shutdownLabel;
    QLabel *systemInfoLabel;
    QLabel *softwareVersionLabel1;
    QLabel *softwareVersionLabel2;

    QSlider *signalStrengthSlider;
    QComboBox *languageComboBox;
    QPushButton *modifyButton;
    QPushButton *rebootButton;
    QPushButton *shutdownButton;
};

#endif // SETTINGSWIDGET_H
