#ifndef SPLASHSCREEN_H
#define SPLASHSCREEN_H

//1) Qt Headers
#include <QSplashScreen>
#include <QLabel>
#include <QProgressBar>
#include <QScreen>

class SplashScreen : public QSplashScreen {
    Q_OBJECT

public:
    explicit SplashScreen(QScreen *pScreen, QWidget *pParent  = nullptr);  // 构造函数

    // 更新状态和进度条
    void UpdateStatus(const QString& strText, int iProgress);

    // 设置进度条最大值
    void SetProgressMax(int iMax);

private:
    QLabel *m_pStatusLabel = nullptr;
    QProgressBar *m_pProgressBar = nullptr;

    void initUI(QScreen *pScreen, const QRect &fullRect);  // 初始化界面
};

#endif // SPLASHSCREEN_H
