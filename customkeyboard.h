#ifndef CUSTOMKEYBOARD_H
#define CUSTOMKEYBOARD_H

#include <QWidget>
#include <QPoint>
#include <QMap>

class QLineEdit;
class QStackedWidget;
class QPushButton;

class CustomKeyboard : public QWidget {
    Q_OBJECT
public:
    // 单例获取实例
    static CustomKeyboard* instance(QWidget *parent = nullptr);

    // 注册输入框及其键盘偏移
    void registerEdit(QLineEdit *edit, const QPoint &offset = QPoint(-35, 0));

signals:
    // 键被按下的信号
    void keyPressed(const QString &key);

protected:
    // 捕获输入框获取焦点事件
    bool eventFilter(QObject *watched, QEvent *event) override;
    // 拖拽相关
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;

private:
    explicit CustomKeyboard(QWidget *parent = nullptr);
    ~CustomKeyboard();

    // 将键盘关联到给定输入框，并按偏移量弹出
    void attachTo(QLineEdit *edit, const QPoint &offset);

    // 构造键盘页面
    QWidget* createNumKeyboard();
    QWidget* createAlphaKeyboard();
    void switchToNumKeyboard();
    void switchToAlphaKeyboard();
    void handleShiftClicked();

    QStackedWidget            *stackedWidget;
    QList<QPushButton*>        letterButtons;
    QMap<QLineEdit*, QPoint>   editOffsetMap;
    QLineEdit                 *currentEdit = nullptr;
    bool                       isUpperCase = false;

    // 拖拽辅助
    bool                       m_dragging;
    QPoint                     m_dragPosition;
};

#endif // CUSTOMKEYBOARD_H
