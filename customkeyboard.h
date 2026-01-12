
/********************************************************************************/
/* 文件名    : CustomKeyboard.h                                                 */
/* 功能      : 自定义虚拟键盘控件（触屏输入）                                   */
/* 版本      : 1.0.0                                                            */
/* 作者      : lxh                                                              */
/* 日期      : 2025-12-29                                                       */
/* 说明      : 提供数字/字母虚拟键盘，支持大小写切换、拖拽、延时隐藏，并可与多个 */
/*            QLineEdit 绑定（焦点触发弹出），采用单例模式全局复用。             */
/********************************************************************************/

#ifndef CUSTOMKEYBOARD_H
#define CUSTOMKEYBOARD_H

#include <QWidget>
#include <QPoint>
#include <QMap>
#include <mutex>

class QLineEdit;
class QStackedWidget;
class QPushButton;

class CustomKeyboard : public QWidget {
    Q_OBJECT
public:
    // 单例获取实例
    static CustomKeyboard* instance(QWidget *parent = nullptr);

    // 注册输入框及其键盘偏移
    void registerEdit(QLineEdit *pEdit, const QPoint &offset = QPoint(-35, 0));

    //下面是单例优化代码（测试）

    // 显示虚拟键盘
    void showKeyboard();

    // 关闭虚拟键盘
    void closeKeyboard();

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

    // 防止拷贝和赋值（单例优化代码，待测试）
    CustomKeyboard(const CustomKeyboard&) = delete;
    CustomKeyboard& operator=(const CustomKeyboard&) = delete;

    // 将键盘关联到给定输入框，并按偏移量弹出
    void attachTo(QLineEdit *edit, const QPoint &offset);    // 将键盘关联到输入框并按偏移量显示

    // 构造键盘页面
    QWidget* createNumKeyboard();    // 构造数字键盘界面
    QWidget* createAlphaKeyboard();  // 构造字母键盘界面
    void switchToNumKeyboard();      // 切换到数字键盘
    void switchToAlphaKeyboard();    // 切换到字母键盘
    void handleShiftClicked();       // 处理 Shift 点击（大小写切换）
    void hideWithDelay();            // 延时隐藏键盘

    QStackedWidget* m_pStackedWidget;   // 存放字母键盘和数字键盘
    QList<QPushButton*> m_lLetterButtons;   // 所有字母按钮（便于大小写切换）
    QMap<QLineEdit*, QPoint> m_mEditOffsetMap;   // 输入框与键盘偏移量映射
    QLineEdit*  m_iCurrentEdit = nullptr;    // 当前获得焦点的输入框
    bool m_bIsUpperCase = false;      // 是否为大写模式

    // 拖拽辅助
    bool m_bDragging;       // 是否正在拖拽
    QPoint m_bDragPosition;    // 拖拽起始位置

    float m_fScaleX;    // X 方向缩放比例
    float m_fScaleY;    // Y 方向缩放比例
};

#endif // CUSTOMKEYBOARD_H
