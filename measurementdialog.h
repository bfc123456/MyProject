#ifndef MEASUREMENTDIALOG_H
#define MEASUREMENTDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>
#include <QDateTime>
#include "measurementtrendwidget.h"
#include "measurementdata.h"  // 包含独立的结构体定义

namespace Ui {
class MeasurementDialog;
}

class MeasurementDialog : public QDialog
{
    Q_OBJECT
public:
    enum State {
        STATE_READY,      // 信号达标，准备测量
        STATE_MEASURING,  // 测量进行中
        STATE_PAUSED,     //暂停
        STATE_COMPLETED   // 测量已完成
    };

    explicit MeasurementDialog(const QString &sensorId, double signalStrength, QWidget *parent = nullptr);
    ~MeasurementDialog() override;
    // 开始测量
    void startMeasurement();

    // 获取测量结果的接口
    const MeasurementData& getCurrentResult() const;
    const QList<MeasurementData>& getHistoryResults() const;

protected:
    void changeEvent(QEvent *event) override;

signals:
    void measurementStarted();
    void measurementCompleted(const MeasurementData& result);
    void measurementPaused(bool paused);
    void measurementRestarted();
    void measurementInterrupted();
    void exitOverlay();
    void closeFollowUpForm();

private slots:
    void updateProgress();
    void onStartButtonClicked();
    void onCancelButtonClicked();
    void onPauseButtonClicked();
    void onRestartButtonClicked();
    void onViewResultButtonClicked();
    float generatePulmonaryPressureSample();    //生成模拟的数据函数
    MeasurementData calculateMeasurementResults();

private:

    // ----统一刷新所有文本的方法 ----
    void retranslateUi();

    QString m_sensorId;
    State currentState =  STATE_READY;
    double initialSignalStrength;
    int progressValue;
    QTimer *progressTimer; // 显式初始化

    // UI组件
    QLabel *titleLabel;
    QLabel *messageLabel;
    QProgressBar *progressBar;
    QPushButton *leftButton;
    QPushButton *rightButton;

    MeasurementData m_currentResult;       // 当前测量结果
    QVector<float> pressureSamples;     //存储数据的容器
    MeasurementTrendWidget *trendWidget;   // 子界面指针
    bool updateResultToDatabase(const MeasurementData &result);
};

#endif // MEASUREMENTDIALOG_H
