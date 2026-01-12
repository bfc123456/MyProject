
/********************************************************************************/
/* 文件名    : CircularProgressBar.h                                            */
/* 功能      : 圆形进度条控件（信号强度显示）                                   */
/* 版本      : 1.0.0                                                            */
/* 作者      : Your Name                                                        */
/* 日期      : 2025-12-29                                                       */
/* 说明      : 自定义圆形进度条，用于显示信号强度及阈值标记                     */
/********************************************************************************/

#ifndef CIRCULARPROGRESSBAR_H
#define CIRCULARPROGRESSBAR_H

//1) Qt Headers
#include <QWidget>

class CircularProgressBar : public QWidget {
    Q_OBJECT
public:
    explicit CircularProgressBar(QWidget *parent=nullptr);

    void setProgress(int iPogress);
    void setThreshold(int iT);     // 新增：设置标准阈值（0-100）
    int  threshold() const { return m_iThreshold; }

protected:
    void paintEvent(QPaintEvent *) override;

private:
    int    m_iProgress  = 0;                  // 实时信号强度
    int    m_iThreshold = 70;                 // 阈值（QSettings 传入）
    QColor m_trackColor = QColor(50, 70, 100);   // 底环颜色
    QColor m_textColor  = QColor(0, 255, 220);   // 中心文字
    QColor m_markColor  = QColor("#FFC857");     // 阈值标记颜色（琥珀黄）
};

#endif // CIRCULARPROGRESSBAR_H
