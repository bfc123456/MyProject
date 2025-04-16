#ifndef CUSTOMKEYBOARD_H
#define CUSTOMKEYBOARD_H

#include <QWidget>
#include <QStackedWidget>
#include <QPushButton>
#include <QPointer>
#include <QLineEdit>
#include <QMap>

class CustomKeyboard : public QWidget
{
    Q_OBJECT
public:
    explicit CustomKeyboard(QWidget *parent = nullptr);
    void attachTo(QLineEdit *edit, const QPoint &offset = QPoint(0, 5));       //附着到目标输入框
    ~CustomKeyboard();
    bool eventFilter(QObject *watched, QEvent *event) override;     //支持自动弹出键盘
    void registerEdit(QLineEdit*edit,const QPoint &offset);     // 注册一个输入框与其偏移量

signals:
    void keyPressed(const QString &text);
    void hideKeyboardRequested();

private slots:
    void handleButtonClicked(const QString &text);
    void switchToNumKeyboard();
    void switchToAlphaKeyboard();
    void handleShiftClicked();

private:
    bool isUpperCase = false;// 用来标记当前是否大写模式
    QStackedWidget *stackedWidget;
    QWidget *createNumKeyboard();
    QWidget *createAlphaKeyboard();
    QList<QPushButton*> letterButtons;
    QPointer<QLineEdit> currentEdit; //当前活跃的输入框
    QMap<QLineEdit*,QPoint> editOffsetMap; //每个输入框对应的偏移位置
};

#endif // CUSTOMKEYBOARD_H
