#ifndef CIRCULARPROGRESSBAR_H
#define CIRCULARPROGRESSBAR_H

#include <QWidget>

class CircularProgressBar : public QWidget
{
    Q_OBJECT

public:
    explicit CircularProgressBar(QWidget *parent = nullptr);

    void setProgress(int progress); // 设置进度
    int progress() const { return m_progress; }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    int m_progress = 0; // 进度值 0-100
};

#endif // CIRCULARPROGRESSBAR_H
