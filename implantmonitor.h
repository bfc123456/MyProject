/********************************************************************************/
/* 文件名    : ImplantMonitor.h                                                 */
/* 功能      : 植入监测界面（医生端）                                           */
/* 版本      : 1.0                                                              */
/* 作者      : Your Name                                                        */
/* 日期      : 2025-12-25                                                       */
/* 说明      : 该文件定义了用于植入监测的界面类，提供了实时数据采集和显示功能   */
/********************************************************************************/

#ifndef IMPLANTMONITOR_H
#define IMPLANTMONITOR_H

#include <QWidget>
#include <QVector>
#include <QPointF>
#include <QTimer>
#include <QElapsedTimer>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QFileDialog>
#include <QMessageBox>

#include "measurementdataprocessor.h"
#include "customexportdatadialog.h"
#include "framelesswindow.h"
#include "global.h"

class ModernWavePlot;
class CircularProgressBar;
class ReadoutRecordDialog;
class BlurOverlayGuard;

class ImplantMonitor : public FramelessWindow
{
    Q_OBJECT

public:
    explicit ImplantMonitor(QWidget* pParent = nullptr, const QString& strSensorId = QString());
    ~ImplantMonitor() override;

protected:
    void changeEvent(QEvent* pEvent) override;

private slots:
    void onStartMeasurement();
    void SlotStopMeasurement();

    // Processor -> UI（实时帧）
    void SlotUpdateWaveformFrame(const QVector<QPointF> &data);

    // Processor -> UI（最终结果）
    void onMeasureFinished(const MeasurementData& result);

    // Worker -> UI（错误）
    void onAcquisitionError(const QString& msg);

    void SlotOpenSaveConfirm();
    void SlotExportCurrentData();
    void SlotOpenSettingsRequested();
    void SlotOnReadoutButtonClicked();
    void SlotOnRowDeleted(int row);
    void SlotOpenCOClicked();     // 心输出量对话框
    void SlotOpenRHCClicked();    // RHC 对话框
    void exportQueuesToFiles(const QQueue<QByteArray>& rawQ, const QQueue<QByteArray>& fftQ);

private:
    void setupDataPipelineOnce();

    void applyMeasuringUiState();
    void applyIdleUiState();

    // 采样率/过滤（修复单位问题）
    double GetSamplingRateHz() const;
    bool GetFilterRangeHz(double& minHz, double& maxHz);

    // 降采样工具（如果你未来继续用批量处理）
    QVector<QPointF> DownsampleData(const QVector<QPointF>& originalData, double targetRateHZ) const;

private:
    float m_fScaleX = 1.0f;
    float m_fScaleY = 1.0f;

    QLabel *m_pTitleLbl = nullptr;
    QLabel *m_pIdLbl = nullptr;

    ModernWavePlot* m_pPlot = nullptr;
    CircularProgressBar* m_pProgressBar = nullptr;

    QLabel* m_pBpValLbl = nullptr;
    QLabel* m_pAvgValLbl = nullptr;
    QLabel* m_pHrValLbl = nullptr;

    QPushButton* m_pStartBtn = nullptr;
    QPushButton* m_pInputCoBtn = nullptr;
    QPushButton* m_pInputRhcBtn = nullptr;
    QPushButton* m_pExportBtn = nullptr;
    QPushButton* m_pStatisticsBtn = nullptr;

    QLabel* sideLabel = nullptr;

    QLabel* m_pSampleRateLabel = nullptr;
    QLineEdit* m_pSampleRateEdit = nullptr;
    QComboBox* m_pSampleRateUnitCombo = nullptr;

    QLabel* m_pFilterRangeLabel = nullptr;
    QCheckBox* m_pFilterEnabledCheckBox = nullptr;
    QLineEdit* m_pRangeStartEdit = nullptr;
    QLineEdit* m_pRangeEndEdit = nullptr;
    QComboBox* m_pFilterUnitCombo = nullptr;

    QLabel* m_pXUnitLabel = nullptr;
    QLabel* m_pYUnitLabel = nullptr;

    // ===== 数据 =====
    QString m_strSensorId;
    bool m_bIsMeasuringFlag = false;
    bool m_bIsLeftFlag = false;

    QVector<QPointF> m_vecPlotPoints;
    QList<MeasurementData> m_lstMeasurements;

    ReadoutRecordDialog* m_pReadoutDialog = nullptr;
    CustomExportDataDialog* m_pExportDataDialog = nullptr;
    BlurOverlayGuard* m_pBlurGuard = nullptr;

    // 你原来用的采样间隔常量（示例：1ms）
    static constexpr double ORIGINAL_SAMPLING_INTERVAL = 0.001;
};

#endif // IMPLANTMONITOR_H
