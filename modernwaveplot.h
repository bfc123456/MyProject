#ifndef MODERN_WAVE_PLOT_H
#define MODERN_WAVE_PLOT_H

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <QVector>
#include <QPointF>

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
