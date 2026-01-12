#include "modernwaveplot.h"
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
    , m_curve(new QwtPlotCurve())
    , m_panner(nullptr)
{
    initPlotStyle();
    initPlotInteraction();

    // 曲线默认配置（抗锯齿、颜色等）
    m_curve->setRenderHint(QwtPlotItem::RenderAntialiased);
    m_curve->setPen(QPen(QColor(0, 170, 255), 2));

    m_curve->setBrush(Qt::NoBrush);
    m_curve->setBaseline(0.0);

    m_curve->attach(this);                                      // 别忘了挂上去
    m_curve->setPaintAttribute(QwtPlotCurve::ClipPolygons, true); // 剪裁填充多边形
}

ModernWavePlot::~ModernWavePlot()
{
    delete m_curve;
    delete m_panner;
}

/**
 * @brief 初始化绘图风格：黑色画布、白色网格/坐标轴、初始范围。
 * @complexity O(1)
 */
void ModernWavePlot::initPlotStyle() {
    setFrameStyle(QFrame::NoFrame);
    plotLayout()->setCanvasMargin(0);

    // 画布样式
    canvas()->setAutoFillBackground(true);
    canvas()->setPalette(QPalette(Qt::black));
    canvas()->setAttribute(Qt::WA_OpaquePaintEvent, true);

    // 网格
    QwtPlotGrid* grid = new QwtPlotGrid();
    grid->setPen(QPen(QColor(255, 255, 255, 15), 1, Qt::DotLine));
    grid->attach(this);

    // 坐标轴字体与颜色
    auto setupAxis = [](QwtScaleWidget* axis) {
        QFont font("Arial", 10);
        axis->setFont(font);
        QPalette pal;
        pal.setColor(QPalette::WindowText, Qt::white);
        axis->setPalette(pal);
    };
    setupAxis(axisWidget(QwtPlot::xBottom));
    setupAxis(axisWidget(QwtPlot::yLeft));

    // 自定义Y轴刻度（整数显示）
    QwtScaleWidget* yAxis = axisWidget(QwtPlot::yLeft);
    CustomYScaleDraw* customYScale = new CustomYScaleDraw(0);
    yAxis->setScaleDraw(customYScale);

    // --------------------------
    // 关键修复：初始化刻度与标题（与成员变量匹配）
    // --------------------------
    // X轴：0~10000ms，标题带单位
    setAxisScale(QwtPlot::xBottom, m_xMin, m_xMax);
    QwtText xTitle(QString("Time (%1)").arg(m_xUnit));
    axisWidget(QwtPlot::xBottom)->setTitle(xTitle);

    // Y轴：0~58e6Hz，标题带单位
    setAxisScale(QwtPlot::yLeft, m_yMin, m_yMax);
    QwtText yTitle(QString("Frequency (%1)").arg(m_yUnit));
    axisWidget(QwtPlot::yLeft)->setTitle(yTitle);
}

// 初始化鼠标交互：左键平移、双击放大、右键重置
// 初始化交互：左键平移、缩放组件（双击放大/右键重置）
void ModernWavePlot::initPlotInteraction()
{
    // 1. 平移组件：正常初始化（不受影响）
    m_panner = new QwtPlotPanner(canvas());
    m_panner->setMouseButton(Qt::LeftButton);
    m_panner->setAxisEnabled(QwtPlot::xBottom, true);
    m_panner->setAxisEnabled(QwtPlot::yLeft, true);

    // 2. 关键修改：将 QWidget* 安全转为 QwtPlotCanvas*
    QwtPlotCanvas* plotCanvas = dynamic_cast<QwtPlotCanvas*>(canvas());
    if (plotCanvas != nullptr) { // 确保转型成功（避免空指针）
        m_zoomer = new CustomZoomer(plotCanvas);
    } else {
        // 异常处理：转型失败时（理论上不会触发，除非 Qwt 版本异常）
        qWarning() << "QwtPlot canvas cast to QwtPlotCanvas failed!";
        m_zoomer = nullptr;
    }

    canvas()->setMouseTracking(true); // 启用鼠标跟踪
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
    m_originalData = data;
    updateScaledCurve(); // 先将数据缩放到当前单位（默认毫秒）

    // 根据模式调整轴范围
    if (m_liveMode) {
        adjustAxesLive();
    } else {
        adjustAxesFull();
    }

    // Live 模式：过滤窗口内数据
    if (m_liveMode) {
        const double left  = axisScaleDiv(QwtPlot::xBottom).lowerBound();
        const double right = axisScaleDiv(QwtPlot::xBottom).upperBound();

        QVector<QPointF> vis;
        vis.reserve(m_currentData.size());
        double lastX = -std::numeric_limits<double>::infinity();

        for (const auto& pt : m_currentData) {
            double x = pt.x();
            if (x < left || x > right) continue;
            if (x <= lastX) x = lastX + 1e-9; // 保证 X 严格递增
            vis.push_back(QPointF(x, pt.y()));
            lastX = x;
        }

        m_curve->setSamples(vis);
    }

    // 设置基线并刷新
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
        // 初始窗口：秒 → 当前单位（默认×1000转毫秒）
        double initialRight = m_windowSec * m_xScaleFactor;
        setAxisScale(QwtPlot::xBottom, 0.0, initialRight);
        return;
    }

    const double xMax = m_currentData.last().x();
    // 窗口宽度：秒 → 当前单位
    double windowSize = m_windowSec * m_xScaleFactor;

    double left, right;
    if (xMax < windowSize) {
        left = 0.0;
        right = windowSize;
    } else {
        left  = xMax - windowSize;
        right = xMax;
    }

    // 计算Y轴极值（原有逻辑不变）
    double yMin = std::numeric_limits<double>::infinity();
    double yMax = -std::numeric_limits<double>::infinity();
    for (const auto& pt : m_currentData) {
        if (pt.x() >= left && pt.x() <= right) {
            yMin = qMin(yMin, pt.y());
            yMax = qMax(yMax, pt.y());
        }
    }
    if (!std::isfinite(yMin) || !std::isfinite(yMax) || yMin == yMax) {
        yMin = (yMin == yMin) ? yMin - 1.0 : 0.0;
        yMax = (yMax == yMax) ? yMax + 1.0 : 1.0;
    }

    // 设置轴范围（带边距）
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

    double xMin = 0.0;
    double xMax = m_currentData.last().x();
    double yMin = m_currentData.first().y();
    double yMax = yMin;

    for (const auto& pt : m_currentData) {
        xMax = qMax(xMax, pt.x()); // 基于缩放后的数据（默认毫秒）
        yMin = qMin(yMin, pt.y());
        yMax = qMax(yMax, pt.y());
    }
    if (yMin == yMax) {
        yMin -= 1;
        yMax += 1;
    }

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
    setAxisScale(QwtPlot::xBottom, DEFAULT_X_MIN, DEFAULT_X_MAX);
    setAxisScale(QwtPlot::yLeft, DEFAULT_Y_MIN, DEFAULT_Y_MAX);
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
    m_curve->setSamples(m_currentData);
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

// 重缩放数据并刷新曲线
void ModernWavePlot::updateScaledCurve() {
    QVector<QPointF> scaledData;
    scaledData.reserve(m_originalData.size());
    for (const auto& pt : m_originalData) {
        // 按当前缩放因子重计算X/Y
        double xScaled = pt.x() * m_xScaleFactor;
        double yScaled = pt.y() * m_yScaleFactor;
        scaledData.append(QPointF(xScaled, yScaled));
    }
    m_currentData = scaledData;
    m_curve->setSamples(m_currentData); // 更新曲线数据
    replot(); // 刷新波形图
}

// X轴单位切换：修复刻度计算与范围有效性
void ModernWavePlot::onXUnitChanged(const QString& unit) {
    m_xUnit = unit;

    // 1. 计算缩放因子（原始数据X为“秒”）
    if (unit == "ms") {
        m_xScaleFactor = 1.0;
    } else if (unit == "s") {
        m_xScaleFactor = 0.001;
    }

    // 2. 计算新刻度范围（优先用初始刻度，数据非空则用数据范围）
    double newXMin, newXMax;
    if (m_currentData.isEmpty()) {
        // 数据为空：用初始刻度转换
        if (unit == "ms") {
            newXMin = m_xMin;       // 0ms
            newXMax = m_xMax;       // 10000ms
        } else if (unit == "s") {
            newXMin = m_xMin / 1000.0; // 0s
            newXMax = m_xMax / 1000.0; // 10s
        }
    } else {
        // 数据非空：用数据的实际范围转换
        double dataXMin = m_currentData.first().x() / m_xScaleFactor;
        double dataXMax = m_currentData.last().x() / m_xScaleFactor;
        newXMin = dataXMin * m_xScaleFactor;
        newXMax = dataXMax * m_xScaleFactor;
    }

    // 3. 设置有效刻度（避免范围异常）
    if (newXMin < newXMax) {
        setAxisScale(QwtPlot::xBottom, newXMin, newXMax);
    } else {
        // 防御：范围无效时用初始值
        setAxisScale(QwtPlot::xBottom, m_xMin, m_xMax);
    }

    // 4. 更新标题与数据
    QwtText xTitle(QString("Time (%1)").arg(unit));
    axisWidget(QwtPlot::xBottom)->setTitle(xTitle);
    updateScaledCurve();
    replot();
}

// Y轴单位切换：同X轴逻辑，确保刻度有效
// Y轴单位切换：同X轴逻辑，确保刻度有效
void ModernWavePlot::onYUnitChanged(const QString& unit) {
    m_yUnit = unit;

    // 1. 计算缩放因子（原始数据Y为“Hz”）
    if (unit == "MHZ") {
        m_yScaleFactor = 1.0; // MHZ 不变
    } else if (unit == "KHZ") {
        m_yScaleFactor = 1000.0; // KHZ -> Hz * 1000
    }

    // 2. 计算新刻度范围
    double newYMin, newYMax;

    // 处理数据为空的情况
    if (m_currentData.isEmpty()) {
        // 如果数据为空，直接使用默认范围
        if (unit == "MHZ") {
            newYMin = m_yMin;       // 默认范围 0 MHz
            newYMax = m_yMax;       // 默认范围 50 MHz
        } else if (unit == "KHZ") {
            newYMin = m_yMin * 1000.0; // 转换为 KHZ
            newYMax = m_yMax * 1000.0; // 转换为 KHZ
        }
    } else {
        // 数据非空：用数据的实际范围转换
        double dataYMin = m_currentData.first().y(); // 直接取原始值
        double dataYMax = m_currentData.last().y();  // 直接取原始值

        // 使用 yScaleFactor 进行转换
        newYMin = dataYMin / m_yScaleFactor;
        newYMax = dataYMax / m_yScaleFactor;
    }

    // 3. 确保刻度有效，避免范围异常
    if (newYMin < newYMax) {
        setAxisScale(QwtPlot::yLeft, newYMin, newYMax); // 设置有效的 Y 轴范围
    } else {
        // 防御：范围无效时恢复到默认值
        setAxisScale(QwtPlot::yLeft, m_yMin, m_yMax);
    }

    // 4. 更新标题与数据
    QwtText yTitle(QString("Frequency (%1)").arg(unit));
    axisWidget(QwtPlot::yLeft)->setTitle(yTitle);

    // 5. 调试输出，检查新刻度
    qDebug() << "New Y axis range: " << newYMin << " - " << newYMax;

    // 6. 更新曲线
    updateScaledCurve();

    // 7. 强制重新绘制图形
    replot();
}
