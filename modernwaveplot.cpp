#include "ModernWaveplot.h"
#include <qwt_plot_canvas.h>
#include <qwt_scale_widget.h>
#include <qwt_plot_grid.h>
#include <QPalette>
#include <QBrush>
#include <QPen>
#include <QFont>
#include <cmath>
#include <qwt_plot_layout.h>
#include <qwt_scale_div.h>

ModernWavePlot::ModernWavePlot(QWidget *parent)
    : QwtPlot(parent)
    , m_curve(new QwtPlotCurve()) // 初始化唯一曲线
{
    initPlotStyle();
    // 曲线默认配置（抗锯齿、颜色等）
    m_curve->setRenderHint(QwtPlotItem::RenderAntialiased);
    m_curve->setPen(QPen(QColor(0, 170, 255), 2));

    m_curve->setBrush(Qt::NoBrush);
    m_curve->setBaseline(0.0);

    m_curve->attach(this);                                      // 别忘了挂上去
    m_curve->setPaintAttribute(QwtPlotCurve::ClipPolygons, true); // 剪裁填充多边形
}

/**
 * @brief 初始化绘图风格：黑色画布、白色网格/坐标轴、初始范围。
 * @complexity O(1)
 */
void ModernWavePlot::initPlotStyle() {
    setFrameStyle(QFrame::NoFrame);
    plotLayout()->setCanvasMargin(0);

    canvas()->setAutoFillBackground(true);
    canvas()->setPalette(QPalette(Qt::black));
    canvas()->setAttribute(Qt::WA_OpaquePaintEvent, true);

    QwtPlotGrid* grid = new QwtPlotGrid();
    grid->setPen(QPen(QColor(255, 255, 255, 15), 1, Qt::DotLine));
    grid->attach(this);

    auto setupAxis = [](QwtScaleWidget* axis) {
        QFont font("Arial", 10);
        axis->setFont(font);
        QPalette pal;
        pal.setColor(QPalette::WindowText, Qt::white);
        axis->setPalette(pal);
    };
    setupAxis(axisWidget(QwtPlot::xBottom));
    setupAxis(axisWidget(QwtPlot::yLeft));

    // 初始轴范围
    setAxisScale(QwtPlot::xBottom, 0, 20);
    setAxisScale(QwtPlot::yLeft, 0, 500);
}


/**
 * @brief 设置当前全量数据并立即刷新。
 * @param data 全量样本点（建议按时间非递减；允许同一时间戳）。
 * @note
 *  - 根据 @ref setLiveMode 设置选择 Live 或 Full 行为：
 *    - Live：先按最新时间构造固定窗口的轴范围，再从 data 裁出窗口点集并保证 X 严格递增，最后 setSamples。
 *    - Full：按全部数据自适应轴范围，再 setSamples 全部数据或窗口数据（实现见 .cpp）。
 *  - 若开启填充，baseline 建议取当前 Y 轴下界（实现中使用）。
 * @complexity O(N)（N 为 data 点数）
 * @thread_safety 必须在 GUI 线程调用。
 */
void ModernWavePlot::setSimpleData(const QVector<QPointF>& data) {
    m_currentData = data; // 覆盖当前数据
    m_curve->setSamples(m_currentData); // 曲线绑定新数据
    // 根据模式调整轴
    if (m_liveMode) {
        adjustAxesLive();  // 实时滚动窗口
    } else {
        adjustAxesFull();  // 全局自适应
    }
    const double left  = axisScaleDiv(QwtPlot::xBottom).lowerBound();
    const double right = axisScaleDiv(QwtPlot::xBottom).upperBound();

    // 只取窗口内的点，且强制 x 递增（同一时刻 +ε）
    QVector<QPointF> vis;
    vis.reserve(m_currentData.size());
    double lastX = -std::numeric_limits<double>::infinity();

    for (const auto& pt : m_currentData) {
        double x = pt.x();
        if (x < left || x > right) continue;
        if (x <= lastX) x = lastX + 1e-9;         // ☆ 关键：严格递增
        vis.push_back(QPointF(x, pt.y()));
        lastX = x;
    }

    m_curve->setSamples(vis);

    // 基线建议用“当前可视 y 下界”或你需要的固定值
    const double baseline = axisScaleDiv(QwtPlot::yLeft).lowerBound();
    m_curve->setBaseline(baseline);

    replot();
}

/**
 * @brief 开关实时滚动模式，并设定窗口宽度。
 * @param on true=Live 模式；false=Full 模式。
 * @param windowSec 实时窗口宽度（秒），建议 >= 0.05s。
 * @note 切换后会立即按新模式重建轴与可见数据并刷新。
 * @complexity O(N)
 * @thread_safety GUI 线程。
 */
void ModernWavePlot::setLiveMode(bool on, double windowSec) {
    m_liveMode  = on;
    if (windowSec > 0.05) m_windowSec = windowSec; // 防御
    // 立即按照当前数据重新设定一次轴
    if (m_currentData.isEmpty()) return;
    if (m_liveMode) adjustAxesLive();
    else            adjustAxesFull();
    replot();
}

/**
 * @brief 实时滚动模式：固定 X 窗口宽度（秒），右对齐最新点。
 * @details 仅按「可见窗口」计算 Y 极值并设置轴范围。
 * @complexity O(N)
 */
void ModernWavePlot::adjustAxesLive() {
    if (m_currentData.isEmpty()) {
        setAxisScale(QwtPlot::xBottom, 0.0, m_windowSec);
        return;
    }

    const double xMax = m_currentData.last().x();
    double left, right;
    if (xMax < m_windowSec) {
        left = 0.0;
        right = m_windowSec;              // 起步阶段固定 8s，不缩放
    } else {
        left  = xMax - m_windowSec;       // 滑动窗口
        right = xMax;
    }

    // 仅对可见窗口内的点求 Y 极值
    double yMin = std::numeric_limits<double>::infinity();
    double yMax = -std::numeric_limits<double>::infinity();
    for (const auto& pt : m_currentData) {
        if (pt.x() >= left && pt.x() <= right) {
            yMin = qMin(yMin, pt.y());
            yMax = qMax(yMax, pt.y());
        }
    }
    if (!std::isfinite(yMin) || !std::isfinite(yMax) || yMin == yMax) {
        // 防御：窗口内没有点或平线
        yMin = (yMin == yMin) ? yMin - 1.0 : 0.0;
        yMax = (yMax == yMax) ? yMax + 1.0 : 1.0;
    }

    const double xMargin = (right - left) * 0.02;
    const double yMargin = (yMax - yMin) * 0.05;
    setAxisScale(QwtPlot::xBottom, left - xMargin, right + xMargin);
    setAxisScale(QwtPlot::yLeft,   yMin - yMargin, yMax + yMargin);
}

/**
 * @brief 全局自适应：按全部数据自适应 X/Y 轴范围。
 * @complexity O(N)
 */
void ModernWavePlot::adjustAxesFull() {
    if (m_currentData.isEmpty()) return;

    double xMin = m_currentData.first().x();
    double xMax = m_currentData.last().x();
    double yMin = m_currentData.first().y();
    double yMax = yMin;

    for (const auto& pt : m_currentData) {
        xMin = qMin(xMin, pt.x());
        xMax = qMax(xMax, pt.x());
        yMin = qMin(yMin, pt.y());
        yMax = qMax(yMax, pt.y());
    }
    if (yMin == yMax) { yMin -= 1; yMax += 1; }

    const double xMargin = (xMax - xMin) * 0.02;
    const double yMargin = (yMax - yMin) * 0.02;

    setAxisScale(QwtPlot::xBottom, xMin - xMargin, xMax + xMargin);
    setAxisScale(QwtPlot::yLeft,   yMin - yMargin, yMax + yMargin);
}

/**
 * @brief 清空数据并恢复初始坐标范围。
 * @complexity O(1)
 * @thread_safety GUI 线程。
 */
// 核心接口2：清空数据（恢复初始状态）
void ModernWavePlot::clearSimpleData() {
    m_currentData.clear(); // 清空容器
    m_curve->setSamples(m_currentData); // 曲线设为空
    // 恢复初始轴范围
    setAxisScale(QwtPlot::xBottom, 0, 20);
    setAxisScale(QwtPlot::yLeft, 0, 500);
    replot(); // 刷新空白界面
}

/**
 * @brief 兼容旧逻辑的简单自适应（内部使用）。
 * @complexity O(N)
 */
void ModernWavePlot::autoAdjustCurrentAxes() {
    if (m_currentData.isEmpty()) return;

    // 计算X/Y轴极值
    double xMin = m_currentData.first().x();
    double xMax = m_currentData.last().x();
    double yMin = m_currentData.first().y();
    double yMax = m_currentData.first().y();

    for (const auto& pt : m_currentData) {
        yMin = qMin(yMin, pt.y());
        yMax = qMax(yMax, pt.y());
    }

    // 避免Y轴极值相同导致显示异常
    if (yMin == yMax) {
        yMin -= 1;
        yMax += 1;
    }

    // 添加2%边距
    double xMargin = (xMax - xMin) * 0.02;
    double yMargin = (yMax - yMin) * 0.02;
    setAxisScale(QwtPlot::xBottom, xMin - xMargin, xMax + xMargin);
    setAxisScale(QwtPlot::yLeft, yMin - yMargin, yMax + yMargin);
}

/**
 * @brief 全局自适应显示全部数据（适合测量结束查看全程）。
 * @note 会基于 m_currentData 重新计算 X/Y 轴范围并刷新。
 * @complexity O(N)
 * @thread_safety GUI 线程。
 */
void ModernWavePlot::showFullSimpleWaveform() {
    adjustAxesFull();  // 基于所有数据调整轴
    replot(); // 刷新显示
}

/**
 * @brief 设置线条颜色与线宽（线宽固定为 2）。
 * @param color 线条颜色。
 * @complexity O(1)
 * @thread_safety GUI 线程。
 */
void ModernWavePlot::setLineColor(const QColor& color) {
    if (m_curve) {
        m_curve->setPen(QPen(color, 2)); // 线宽固定为2，可按需调整
    }
}

/**
 * @brief 设置“曲线下方填充”的颜色与基线。
 * @param fill 填充颜色（半透明推荐）。
 * @param baseline 填充基线（常用 0 或当前 Y 轴下界）。
 * @note 实时模式下大量点填充可能影响性能；若出现日志
 *       "Polygon too complex for filling."，请只给窗口点并保证 X 严格递增（本类已处理）。
 * @complexity O(1)
 * @thread_safety GUI 线程。
 */
void ModernWavePlot::setFillColor(const QColor& fill, double baseline) {
    if (m_curve) {
        m_curve->setBrush(QBrush(fill));   // 填充画刷
        m_curve->setBaseline(baseline);    // 填充基线
    }
}
