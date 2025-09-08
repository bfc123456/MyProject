
#ifndef CIRCULARPROGRESSBAR_H
#define CIRCULARPROGRESSBAR_H

#include <QWidget>

/**
 * @class CircularProgressBar
 * @brief 圆形进度条控件（信号强度显示）
 *
 * 本控件是一个自定义的圆形进度条，主要用于在界面中直观显示信号强度，
 * 通过圆环的填充比例和颜色变化，反映实时的信号强弱。
 *
 * 功能特点：
 * - 支持设置信号强度值（0~100）
 * - 通过圆形渐变填充，直观表示强度大小
 * - 可自定义颜色、边框宽度、字体样式等外观
 * - 适合集成到 ImplantMonitor 等测量界面，用于实时反馈传感器信号质量
 *
 * @note 该控件主要用于医疗测量场景的 UI 展示，不涉及具体的数据采集逻辑。
 */


class CircularProgressBar : public QWidget {
    Q_OBJECT
public:
    explicit CircularProgressBar(QWidget *parent=nullptr);

    void setProgress(int progress);
    void setThreshold(int t);     // 新增：设置标准阈值（0-100）
    int  threshold() const { return m_threshold; }

protected:
    void paintEvent(QPaintEvent *) override;

private:
    int    m_progress  = 0;                  // 实时信号强度
    int    m_threshold = 70;                 // 阈值（QSettings 传入）
    QColor m_trackColor = QColor(50, 70, 100);   // 底环颜色
    QColor m_textColor  = QColor(0, 255, 220);   // 中心文字
    QColor m_markColor  = QColor("#FFC857");     // 阈值标记颜色（琥珀黄）
};

#endif // CIRCULARPROGRESSBAR_H
