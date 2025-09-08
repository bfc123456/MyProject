#include "CircularProgressbar.h"
#include <QPainter>
#include <QtMath>

CircularProgressBar::CircularProgressBar(QWidget *parent)
    : QWidget(parent), m_progress(0) // 初始化进度为 0
{
    setMinimumSize(100, 100); // 设置最小尺寸，避免过小导致绘制变形
}

static inline int deg16(double deg) { return static_cast<int>(deg * 16.0); }

void CircularProgressBar::setProgress(int progress)
{
    // 将 progress 限制在 0 ~ 100 之间
    m_progress = qBound(0, progress, 100);
    update(); // 请求重新绘制（触发 paintEvent）
}

void CircularProgressBar::paintEvent(QPaintEvent *)
{
    const int side = qMin(width(), height());
    const int thickness = qMax(6, static_cast<int>(side * 0.10)); // 圆环宽度≈10%
    const int pad = thickness;

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.translate(width() / 2.0, height() / 2.0);

    const QRectF arcRect(-side/2.0 + pad, -side/2.0 + pad,
                         side - 2.0*pad,  side - 2.0*pad);

    // 1) 底环
    QPen pen(m_trackColor, thickness, Qt::SolidLine, Qt::FlatCap);
    p.setPen(pen);
    const int start = deg16(90);   // 以 12 点方向为起点
    p.drawArc(arcRect, start, deg16(-360));

    // 2) 渐变进度弧
    QConicalGradient gradient(0, 0, 90); // 从顶部开始
    gradient.setColorAt(0.00, QColor(0, 220, 180));
    gradient.setColorAt(1.00, QColor(0, 180, 150));
    QPen progressPen(QBrush(gradient), thickness, Qt::SolidLine, Qt::FlatCap);
    p.setPen(progressPen);
    const int span = deg16(-3.6 * m_progress); // 顺时针为负
    p.drawArc(arcRect, start, span);

    // 3) 阈值标记（短弧风格：更温和）
//    {
//        const double markDeg = 3.5;                     // 标记弧长（度）
//        const double thDegFromTop = 360.0 * (m_threshold / 100.0);
//        const int thStart = deg16(90.0 - thDegFromTop); // 从顶部沿顺时针
//        QPen thPen(m_markColor, thickness, Qt::SolidLine, Qt::FlatCap);
//        p.setPen(thPen);
//        p.drawArc(arcRect, thStart, deg16(-markDeg));
//    }
    // 若想“刻度线”风格，改为下面这段，替换上面的短弧块：
    {
        const double radius = arcRect.width()/2.0;
        const double thDegFromTop = 360.0 * (m_threshold / 100.0);
        const double rad = qDegreesToRadians(90.0 - thDegFromTop);
        QPointF c(0,0);
        QPointF a(c.x() + (radius - thickness*0.20) * std::cos(rad),
                  c.y() - (radius - thickness*0.20) * std::sin(rad));
        QPointF b(c.x() + (radius + thickness*0.20) * std::cos(rad),
                  c.y() - (radius + thickness*0.20) * std::sin(rad));
        QPen tickPen(m_markColor, 4, Qt::SolidLine, Qt::RoundCap);
        p.setPen(tickPen);
        p.drawLine(a, b);
    }

    // 4) 中央文字
    p.setPen(m_textColor);
    QFont font("Arial", qMax(10, static_cast<int>(side * 0.18)), QFont::Bold);
    p.setFont(font);
    const QString text = QString::number(m_progress) + "%";
    p.drawText(QRectF(-side/2.0, -side/2.0, side, side), Qt::AlignCenter, text);
}

void CircularProgressBar::setThreshold(int t)
{
    m_threshold = qBound(0, t, 100);
    update();                        // 触发重绘（如果你画了阈值刻度）
}

