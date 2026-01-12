
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
#include "framelesswindow.h"

/**
 * @class ReviewWidget
 * @brief 测量数据复核窗口
 *
 * ReviewWidget 提供对单个传感器 ID 下的历史测量数据进行
 * 复核、浏览与保存的功能。用户可在此界面查看血压、心率等
 * 历史记录，浏览波形趋势，并选择是否上传到数据库。
 *
 * 功能：
 * - setDataList()：加载并显示历史测量数据列表。
 * - 展示血压波形（ModernWavePlot）及关键指标（收缩压、舒张压、心率）。
 * - 支持保存/上传数据库（uploadToDatabase()）。
 * - 提供返回主窗口、返回植入监测界面的交互入口。
 * - 支持多语言切换（重写 changeEvent()）。
 *
 * 信号：
 * - returnToMainWindowSignal()：返回主窗口。
 * - returnToImplantmonitor()：返回植入监测界面。
 *
 * 槽函数：
 * - showExitConfirmWidget()：显示退出确认对话框。
 * - OpenSettingsRequested()：请求打开设置窗口。
 * - uploadToDatabase()：执行数据上传操作。
 *
 * 内部成员：
 * - SettingsWidget *settingswidget：设置窗口指针。
 * - QTableWidget *historyTable：历史测量记录表格。
 * - ModernWavePlot *bpPlot：血压波形显示控件。
 * - QList<MeasurementData> m_list：当前加载的数据列表。
 * - 其它 QLabel/QPushButton：用于标题、指标、返回/保存操作。
 */

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
