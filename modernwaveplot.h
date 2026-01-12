#ifndef MODERN_WAVE_PLOT_H
#define MODERN_WAVE_PLOT_H

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <QVector>
#include <QPointF>
#include <qwt_plot_panner.h>
#include <qwt_event_pattern.h>
#include "customyscaledraw.h"
#include "customzoomer.h"

/**
 * @class ModernWavePlot
 * @brief 现代化单通道波形绘制控件（基于 QwtPlot）
 *
 * 该类封装了一个轻量化的波形显示控件，主要用于医疗/科学测量中的
 * 单通道实时数据可视化。支持 **实时滚动模式** 与 **全局显示模式**，
 * 提供接口控制曲线颜色、填充效果等。
 *
 * 特点：
 * - 内部仅维护一个 QwtPlotCurve，适合单通道波形。
 * - 支持 setSimpleData() 一次性覆盖数据并刷新。
 * - 支持实时模式：窗口宽度可调（秒级），实现类似监护仪的滚动效果。
 * - 支持全局模式：自动缩放坐标轴以适应所有数据。
 * - 提供外观接口（颜色、填充、基线）便于快速定制 UI 风格。
 *
 * 典型用法：
 * @code
 * ModernWavePlot *plot = new ModernWavePlot(this);
 * plot->setLineColor(Qt::green);
 * plot->setLiveMode(true, 10.0); // 开启10秒滚动窗口
 * plot->setSimpleData(wavePoints); // 输入波形数据
 * @endcode
 */
#define DEFAULT_X_MIN 0
#define DEFAULT_X_MAX 30
#define DEFAULT_Y_MIN 0
#define DEFAULT_Y_MAX 100

class ModernWavePlot : public QwtPlot {
    Q_OBJECT
public:
    explicit ModernWavePlot(QWidget *parent = nullptr);
    ~ModernWavePlot();

    // 数据接口（GUI UI线程调用）
    void setSimpleData(const QVector<QPointF>& data);   //覆盖当前数据并刷新
    void clearSimpleData(); //清空并恢复默认轴
    void showFullSimpleWaveform();  //全局自适应显示
    void setLiveMode(bool on, double windowSec = 8.0);  //实时窗口滚动模式

    //外观接口
    void setLineColor(const QColor& color);      // 设置曲线颜色
    void setFillColor(const QColor& fill, double baseline = 0.0); // 设置填充

public slots:
    // X轴单位切换（如 s → ms → μs）
    void onXUnitChanged(const QString& unit);
    // Y轴单位切换（如 MHz → kHz → Hz）
    void onYUnitChanged(const QString& unit);

private:
    void initPlotStyle(); // 保留原样式初始化
    void initPlotInteraction();//初始化鼠标在交互
    QwtPlotCurve* m_curve; // 仅一个曲线对象，无需会话
    QVector<QPointF> m_currentData; // 仅一个容器存当前数据
    void adjustAxesLive();   // 实时滚动模式
    void adjustAxesFull();   // 全程自适应
    void autoAdjustCurrentAxes();   //自动调整

private:
    bool   m_liveMode = false;
    double m_windowSec = 8.0;  // 实时窗口宽度（秒）
    QwtPlotPanner* m_panner; // 鼠标平移组件
    CustomZoomer* m_zoomer; // 缩放（双击/右键）组件
    QVector<QPointF> m_originalData; // 存储原始基础单位数据（X:秒，Y:MHz）
    void updateScaledCurve();        // 重缩放数据并刷新曲线

    // 关键补充：单位与刻度参数
    QString m_xUnit = "ms";          // X轴当前单位（默认毫秒）
    double m_xScaleFactor = 1000.0;  // X轴缩放因子（秒→毫秒×1000）
    double m_xMin = 0.0;             // X轴初始最小值（默认0）
    double m_xMax = 10000.0;         // X轴初始最大值（默认10000ms）

    QString m_yUnit = "MHZ";          // Y轴当前单位（默认兆赫兹）
    double m_yScaleFactor = 1.0;     // Y轴缩放因子（MHz不缩放）
    double m_yMin = 0.0;             // Y轴初始最小值（默认0）
    double m_yMax = 50.0;       // Y轴初始最大值（默认50MHz）

};

#endif // MODERN_WAVE_PLOT_H
