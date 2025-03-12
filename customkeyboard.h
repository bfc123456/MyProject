// customkeyboard.h
#ifndef CUSTOMKEYBOARD_H
#define CUSTOMKEYBOARD_H

#include <QWidget>
#include <QStackedWidget>
#include <QPushButton>

class QStackedWidget;

class CustomKeyboard : public QWidget
{
    Q_OBJECT
public:
    explicit CustomKeyboard(QWidget *parent = nullptr);

signals:
    void keyPressed(const QString &text);
    void hideKeyboardRequested();

private slots:
    void handleButtonClicked(const QString &text);
    // 注意这里的函数名
    void switchToNumKeyboard();
    void switchToAlphaKeyboard();
    void handleShiftClicked();

private:
    // 用来标记当前是否大写模式
    bool isUpperCase = false;
    QStackedWidget *stackedWidget;
    QWidget *createNumKeyboard();
    QWidget *createAlphaKeyboard();

    // 为了方便更新字母大小写，保存字母按钮指针
    QList<QPushButton*> letterButtons;

};

#endif // CUSTOMKEYBOARD_H
