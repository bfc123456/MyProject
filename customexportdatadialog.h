/******************************************************************************
 * 文件名：customexportdatadialog.h
 * 功能描述：定义数据导出对话框类及后台工作线程类，负责多线程文件 IO 调度与 UI 交互。
 * 版本：v1.0.0
 * 作者：
 * 时间：2026/01/15
 * 备注：支持分辨率适配与深色现代化 QSS 样式。
 ******************************************************************************/

#ifndef CUSTOMEXPORTDATADIALOG_H
#define CUSTOMEXPORTDATADIALOG_H

#include "closeonlywindow.h"
#include <QThread>
#include <QQueue>
#include <QByteArray>
#include <QRadioButton>
#include <QButtonGroup>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include <QFrame>

/**
 * @brief 导出数据类型枚举
 */
enum class CustomExportType {
    RawOnly = 0,
    FftOnly = 1,
    Both = 2
};

/**
 * @class CustomExportWorker
 * @brief 负责在子线程中执行具体的文件写入操作，避免阻塞 UI。
 */
class CustomExportWorker : public QObject {
    Q_OBJECT
public:
    // 修改构造函数声明
    CustomExportWorker(const QString &path, CustomExportType type,
                           const QVector<quint16> adc, const QVector<quint32> fft);

public slots:
    void process(); ///< 执行导出任务的主逻辑槽函数

signals:
    void progressChanged(int value);             ///< 导出进度汇报 (0-100)
    void finished(bool success, const QString &msg); ///< 任务完成信号

private:
    QString m_savePath;
    CustomExportType m_type;
    QVector<quint16> m_adcData; // 存储纯净 ADC 数值
    QVector<quint32> m_fftData; // 存储纯净 FFT 数值
};

/**
 * @class CustomExportDataDialog
 * @brief 导出交互对话框类，继承自 CloseOnlyWindow。
 * @note 实现分辨率自适应、遮罩模糊关联及数据封送逻辑。
 */
class CustomExportDataDialog : public CloseOnlyWindow {
    Q_OBJECT

public:
    explicit CustomExportDataDialog(QWidget *parent = nullptr);
    ~CustomExportDataDialog();

    /**
     * @brief 外部接口：接收主线程封送的数据并启动后台导出线程
     */
    void startExportTask(const QVector<quint16>& adcData, const QVector<quint32>& fftData);

signals:
    /**
     * @brief 信号：请求主线程准备数据
     */
    void requestData();

private slots:
    void onBrowseClicked();                       ///< 浏览文件夹
    void onStartClicked();                        ///< 点击开始按钮
    void onProgressUpdate(int value);             ///< 刷新进度条
    void onExportFinished(bool success, const QString &msg); ///< 处理结束逻辑

private:
    void setupUi();            ///< 初始化布局与控件
    void applyModernStyle();   ///< 加载 QSS 样式表

    QString getSavePath() const;           ///< 获取界面输入的路径
    CustomExportType getSelectedType() const; ///< 获取当前选中的导出模式

private:
    // 分辨率缩放因子
    float m_pScaleX, m_pScaleY;

    // UI 容器与控件指针
    QFrame       *m_pCardFrame;
    QButtonGroup *m_pTypeGroup;
    QRadioButton *m_pRbRaw, *m_pRbFft, *m_pRbBoth;
    QLineEdit    *m_pPathEdit;
    QPushButton  *m_pBrowseBtn, *m_pStartBtn, *m_pCancelBtn;
    QLabel       *m_pStatusLabel, *m_pTitleLbl;
    QProgressBar *m_pProgressBar;

    // 线程管理
    QThread      *m_pWorkerThread;
};

#endif // CUSTOMEXPORTDATADIALOG_H
