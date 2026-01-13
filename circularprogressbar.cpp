/********************************************************************************/
/* 文件名    : CircularProgressBar.cpp                                          */
/* 功能      : 圆形进度条控件（信号强度显示）                                   */
/* 版本      : 1.0.1                                                            */
/* 日期      : 2025-12-29                                                       */
/* 说明      : 自定义圆形进度条类实现：底环 + 渐变进度弧 + 阈值刻度 + 居中文字     */
/********************************************************************************/

#include "circularprogressbar.h"

#include <QPainter>
#include <QFontMetrics>
#include <QtMath>
#include <algorithm>

// Qt drawArc 角度单位：1/16 度
static inline int deg16(double deg) { return static_cast<int>(deg * 16.0); }

/**
 * @brief 根据目标矩形自动选择合适字号（像素字号更稳定，适配不同缩放/分辨率）
 * @param sample    用来测量的样本文本（建议用最大宽高的字符串，如 "100%"）
 * @param rect      允许绘制文字的区域
 * @param family    字体
 * @param bold      是否加粗
 * @param margin    安全系数，0.9~0.98 之间越小越保守
 */
static QFont fitFontToRectPx(const QString& sample,
                             const QRectF& rect,
                             const QString& family = "Arial",
                             bool bold = true,
                             double margin = 0.95)
{
    QFont f(family);
    f.setBold(bold);

    int lo = 6;
    int hi = 400;
    int best = lo;

    while (lo <= hi) {
        const int mid = (lo + hi) / 2;
        f.setPixelSize(mid);
        const QFontMetrics fm(f);
        const QRect br = fm.boundingRect(sample);

        if (br.width() <= rect.width() * margin &&
            br.height() <= rect.height() * margin) {
            best = mid;
            lo = mid + 1;
        } else {
            hi = mid - 1;
        }
    }

    f.setPixelSize(best);
    return f;
}

CircularProgressBar::CircularProgressBar(QWidget *parent)
    : QWidget(parent)
    , m_iProgress(0)
    , m_iThreshold(50) // 给个默认阈值，避免未初始化
{
    setMinimumSize(100, 100);
}

void CircularProgressBar::setProgress(int progress)
{
    m_iProgress = qBound(0, progress, 100);
    update();
}

void CircularProgressBar::setThreshold(int t)
{
    m_iThreshold = qBound(0, t, 100);
    update();
}

void CircularProgressBar::paintEvent(QPaintEvent *)
{
    // ========== 0) 统一尺寸参数（只算一次） ==========
    const int side      = qMin(width(), height());
    const int thickness = qMax(6, static_cast<int>(side * 0.10)); // 环宽约 10%
    const int pad       = thickness;

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.translate(width() / 2.0, height() / 2.0);

    const QRectF arcRect(-side/2.0 + pad, -side/2.0 + pad,
                         side - 2.0*pad,  side - 2.0*pad);

    const int start = deg16(90);  // 12 点方向为起点（Qt里正角度是逆时针）

    // ========== 1) 底环 ==========
    {
        QPen pen(m_trackColor, thickness, Qt::SolidLine, Qt::FlatCap);
        p.setPen(pen);
        p.drawArc(arcRect, start, deg16(-360));
    }

    // ========== 2) 进度弧 ==========
    {
        QConicalGradient gradient(0, 0, 90);
        gradient.setColorAt(0.00, QColor(0, 220, 180));
        gradient.setColorAt(1.00, QColor(0, 180, 150));

        QPen progressPen(QBrush(gradient), thickness, Qt::SolidLine, Qt::FlatCap);
        p.setPen(progressPen);

        const int span = deg16(-3.6 * m_iProgress); // 顺时针为负
        p.drawArc(arcRect, start, span);
    }

    // ========== 3) 阈值刻度线 ==========
    {
        const double radius = arcRect.width() / 2.0;
        const double thDegFromTop = 360.0 * (m_iThreshold / 100.0);
        const double rad = qDegreesToRadians(90.0 - thDegFromTop);

        const QPointF c(0, 0);

        // 刻度线长度跟 thickness 挂钩，视觉随尺寸变化
        const double inner = radius - thickness * 0.25;
        const double outer = radius + thickness * 0.25;

        const QPointF a(c.x() + inner * std::cos(rad),
                        c.y() - inner * std::sin(rad));
        const QPointF b(c.x() + outer * std::cos(rad),
                        c.y() - outer * std::sin(rad));

        QPen tickPen(m_markColor, qMax(3, thickness / 4), Qt::SolidLine, Qt::RoundCap);
        p.setPen(tickPen);
        p.drawLine(a, b);
    }

    // ========== 4) 中央文字（字号随圆环内径自适配） ==========
    {
        const QString text = QString::number(m_iProgress) + "%";

        // 给文字留“安全边距”，避免触碰到环
        const QRectF innerRect = arcRect.adjusted(
            thickness * 0.90,
            thickness * 0.90,
           -thickness * 0.90,
           -thickness * 0.90
        );

        // 用最大宽度样本来求字号，避免 9% → 100% 时溢出
        QFont font = fitFontToRectPx("100%", innerRect, "Arial", true, 0.95);

        p.setPen(m_textColor);
        p.setFont(font);
        p.drawText(innerRect, Qt::AlignCenter, text);
    }
}
