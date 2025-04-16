#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QElapsedTimer>
#include <QTimer>
#include <QDebug>
#include <QTranslator>
#include <QElapsedTimer>
#include "maintenance.h"
#include "followupform.h"
#include "patientlistwidget.h"
#include "implantinfowidget.h"
#include <memory>

class LoginWindow : public QWidget {
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow();
    void changeLanguage(const QString &languageCode);
    void openSettingsWindow();

protected:
    void paintEvent(QPaintEvent *event) override;
    void closeEvent(QCloseEvent *event) override;  // 重载 closeEvent 方法

private:
    QVBoxLayout *mainLayout;
    QLabel *titleLabel;
    QPushButton *btnNewPerson;
    QPushButton *btnVisit;
    QPushButton *btnPatientList;

    QElapsedTimer clickTimer; //连续点击记录器
    MaintenanceWidget  *hiddenWidget = nullptr;//隐藏界面
    QTranslator translator;//翻译器

    void showHiddenWidget();
    void changeEvent(QEvent *event) override;

    bool isInitialized = false; //初始化标志位

    std::unique_ptr<ImplantInfoWidget> implantWindow;
    std::unique_ptr<FollowUpForm> followupformwindow;
    std::unique_ptr<PatientListWidget> patientlistwidget;

private slots:
    void onSettingClicked();
    void openImplantWindow();
    void closeImplantWindow();
    void showLoginWindow();
    void openFollowupFormWindow();
    void closeFollowupformwindow();
    void openPatientListWidget();
    void closePatientListWidget();
};


#endif // LOGINWINDOW_H
