#include "CircularProgressBar.h"
#include <QPainter>
#include <QConicalGradient>
#include <QFont>

CircularProgressBar::CircularProgressBar(QWidget *parent)
    : QWidget(parent), m_progress(0)
{
    setMinimumSize(100, 100);
}

void CircularProgressBar::setProgress(int progress)
{
    m_progress = qBound(0, progress, 100);
    update();
}

void CircularProgressBar::paintEvent(QPaintEvent *)
{
    int side = qMin(width(), height());
    int thickness = side * 0.1;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.translate(width() / 2, height() / 2);

    // 1. 底部灰色圆环（更深色以适配深蓝背景）
    QPen basePen(QColor(50, 70, 100), thickness, Qt::SolidLine, Qt::FlatCap);
    painter.setPen(basePen);
    painter.drawArc(QRectF(-side / 2 + thickness, -side / 2 + thickness,
                           side - thickness * 2, side - thickness * 2),
                    0, 360 * 16);

    // 2. 渐变进度条（科技绿-青色渐变）
    QConicalGradient gradient(0, 0, 90); // 从顶部开始
    gradient.setColorAt(0.0, QColor(0, 220, 180));
    gradient.setColorAt(1.0, QColor(0, 180, 150));

    QPen progressPen(QBrush(gradient), thickness, Qt::SolidLine, Qt::FlatCap);
    painter.setPen(progressPen);
    painter.drawArc(QRectF(-side / 2 + thickness, -side / 2 + thickness,
                           side - thickness * 2, side - thickness * 2),
                    90 * 16, -m_progress * 3.6 * 16);

    // 3. 中心文字
    painter.setPen(QColor(0, 255, 220)); // 柔和科技绿字体
    QFont font("Arial", side * 0.18, QFont::Bold);
    painter.setFont(font);
    QString progressText = QString("%1%").arg(m_progress);
    painter.drawText(QRectF(-side / 2, -side / 2, side, side), Qt::AlignCenter, progressText);
}
