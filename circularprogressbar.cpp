#include "CircularProgressBar.h"
#include <QPainter>
#include <QConicalGradient>
#include <QFont>

CircularProgressBar::CircularProgressBar(QWidget *parent)
    : QWidget(parent), m_progress(0)
{
    setMinimumSize(100, 100); // 设定最小尺寸，防止绘制过小
}

void CircularProgressBar::setProgress(int progress)
{
    if (progress < 0) progress = 0;
    if (progress > 100) progress = 100;

    m_progress = progress;
    update(); // 重新绘制
}

void CircularProgressBar::paintEvent(QPaintEvent *)
{
    // 画布大小
    int side = qMin(width(), height());
    int thickness = side * 0.1; // 进度条厚度

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing); // 开启抗锯齿
    painter.translate(width() / 2, height() / 2); // 画布中心

    // 1. **绘制底部灰色圆环**
    QPen basePen(QColor(220, 220, 220), thickness, Qt::SolidLine, Qt::FlatCap);
    painter.setPen(basePen);
    painter.drawArc(QRectF(-side / 2 + thickness, -side / 2 + thickness,
                           side - thickness * 2, side - thickness * 2),
                    0, 360 * 16);

    // 2. **绘制带有渐变色的圆形进度条**
    QPen progressPen(Qt::NoPen);
    painter.setPen(progressPen);

    QConicalGradient gradient(0, 0, 90); // 90°开始渐变
    gradient.setColorAt(0.0, QColor(0, 255, 0));    // 绿色
    gradient.setColorAt(1.0, QColor(200, 255, 200)); // 渐变到浅绿

    painter.setBrush(QBrush(gradient));

    QRectF progressRect(-side / 2 + thickness, -side / 2 + thickness,
                        side - thickness * 2, side - thickness * 2);

    painter.drawPie(progressRect, 90 * 16, -m_progress * 3.6 * 16); // -m_progress 逆时针旋转

    // 3. **绘制进度文本**
    painter.setPen(Qt::blue);
    QFont font("Arial", side * 0.15, QFont::Bold);
    painter.setFont(font);

    QString progressText = QString("%1%").arg(m_progress);
    painter.drawText(progressRect, Qt::AlignCenter, progressText);
}
