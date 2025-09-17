#ifndef MODERN_WAVE_PLOT_H
#define MODERN_WAVE_PLOT_H

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <QVector>
#include <QPointF>

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

class ModernWavePlot : public QwtPlot {
    Q_OBJECT
public:
    explicit ModernWavePlot(QWidget *parent = nullptr);
    ~ModernWavePlot() = default;

    // 数据接口（GUI UI线程调用）
    void setSimpleData(const QVector<QPointF>& data);   //覆盖当前数据并刷新
    void clearSimpleData(); //清空并恢复默认轴
    void showFullSimpleWaveform();  //全局自适应显示
    void setLiveMode(bool on, double windowSec = 8.0);  //实时窗口滚动模式

    //外观接口
    void setLineColor(const QColor& color);      // 设置曲线颜色
    void setFillColor(const QColor& fill, double baseline = 0.0); // 设置填充

private:
    void initPlotStyle(); // 保留原样式初始化
    QwtPlotCurve* m_curve; // 仅一个曲线对象，无需会话
    QVector<QPointF> m_currentData; // 仅一个容器存当前数据
    void adjustAxesLive();   // 实时滚动模式
    void adjustAxesFull();   // 全程自适应
    void autoAdjustCurrentAxes();   //自动调整

private:
    bool   m_liveMode = false;
    double m_windowSec = 8.0;  // 实时窗口宽度（秒）

};

#endif // MODERN_WAVE_PLOT_H
