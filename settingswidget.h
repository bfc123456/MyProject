#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QComboBox>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include "mainwindow.h"

class SettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsWidget(MainWindow *parentWindow,QWidget *parent = nullptr);
    ~SettingsWidget();

private:
    void createUI();  // 创建UI函数

    //上部部分控件
    QSlider *signalStrengthSlider;
    QPushButton *modifyButton;
    QPushButton *rebootButton;
    QPushButton *shutdownButton;
    QComboBox *languageComboBox;
    QTextEdit *systemInfoText;
    QLabel *systemSettingsLabel;
    QLabel *signalStrengthLabel;
    QLabel *languageLabel;
    QLabel *softwareVersionLabel;
    QLabel *rebootLabel;
    QLabel *shutdownLabel;
    QLabel *systemInfoLabel;
    QLabel *softwareVersionLabel1;
    QLabel *softwareVersionLabel2;//版本号

    Ui::MainWindow *ui;

    void onRebootClicked();

    MainWindow *mainWindow;//指向 MainWindow
    void changeEvent(QEvent *event) override;
private slots:
    void onLanguageChanged(const QString &);  // 语言切换槽函数
    void onShutdownClicked();

};

#endif // SETTINGSWIDGET_H
