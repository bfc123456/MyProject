#include "touchdateedit.h"

#include <QCalendarWidget>
#include <QEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QStyle>
#include <QStyleOptionSpinBox>
#include <QTextCharFormat>
#include <QBrush>
#include <QColor>

TouchDateEdit::TouchDateEdit(QWidget *parent)
    : QDateEdit(parent)
{
    setCalendarPopup(true);

    // 触控：单击任意位置弹出
    m_openOnClick = true;
    installEventFilter(this);
}

TouchDateEdit::~TouchDateEdit() = default;

void TouchDateEdit::setOpenOnClick(bool enable)
{
    m_openOnClick = enable;
}

void TouchDateEdit::applyTouchStyle(float scaleX, float scaleY,
                                    int popupWidth, int popupHeight,
                                    int editFontPt, int calFontPt)
{
    // ===== 编辑框字体/高度 =====
    QFont editFont = font();
    editFont.setPointSize(editFontPt);
    setFont(editFont);
    setMinimumHeight(static_cast<int>(56 * scaleY));

    // ===== 编辑框样式：保持你“深蓝输入框”体系，并带右侧按钮区 =====
    setStyleSheet(R"(
        QDateEdit{
            padding-left: 12px;
            padding-right: 46px;
            color: white;
            background-color: qlineargradient(x1:0,y1:0,x2:0,y2:1,
                          stop:0 #32495d, stop:1 #1f2e3b);
            border: 1px solid #3e5261;
            border-radius: 6px;
            font-size: 14px;
        }
        QDateEdit:focus{
            border: 1px solid #66bbee;
            background-color: #2c3e4f;
        }

        QDateEdit::drop-down{
            subcontrol-origin: padding;
            subcontrol-position: top right;
            width: 46px;
            border-left: 1px solid rgba(255,255,255,0.12);
            background: rgba(255,255,255,0.06);
        }
        QDateEdit::drop-down:hover{
            background: rgba(255,255,255,0.10);
        }

        /* 这里的图片路径如果不存在，会导致编译/运行问题（你的报错“no rule to make target”）
           如果 ARM/工程资源不稳定，建议先注释掉 image 行，用纯样式也能用 */
        QDateEdit::down-arrow{
            image: url(:/image/icons8-dateedit-96.png);
            width: 18px;
            height: 18px;
        }
    )");

    // ===== 日历弹窗（浅灰哑光 + 周标题黑/红）=====
    if (QCalendarWidget *calendar = calendarWidget()) {
        calendar->setGridVisible(true);
        calendar->setFixedSize(int(popupWidth * scaleX), int(popupHeight * scaleY));

        QFont calFont = calendar->font();
        calFont.setPointSize(calFontPt);
        calendar->setFont(calFont);

        calendar->setStyleSheet(R"(
            QCalendarWidget{
                background-color: #2B2F38;
                border: 1px solid rgba(255,255,255,60);
                border-radius: 10px;
            }

            /* 顶部年月导航栏按钮 */
            QCalendarWidget QToolButton{
                color: white;
                font-size: 18px;
                font-weight: 700;
                padding: 12px 16px;
                background: transparent;
                border: none;
            }
            QCalendarWidget QToolButton:hover{
                background: rgba(255,255,255,30);
                border-radius: 8px;
            }

            /* 星期标题行：背景浅灰哑光 */
            QCalendarWidget QHeaderView::section{
                background: #E6E5E2;
                color: #111111;
                font-size: 18px;
                font-weight: 700;
                padding: 14px 0px;
                border: 1px solid #222222;
            }

            /* 日期网格：背景浅灰哑光、日期字体稍小一点 */
            QCalendarWidget QAbstractItemView{
                background: #E6E5E2;
                color: #111111;
                font-size: 14px;
                selection-background-color: #1E8CFF;
                selection-color: white;
                outline: 0;
                border: none;
            }

            QCalendarWidget QAbstractItemView::item{
                border: 1px solid #222222;
                padding: 6px;
                margin: 0px;
            }
            QCalendarWidget QAbstractItemView::item:hover{
                background: rgba(30,140,255,35);
            }
            QCalendarWidget QAbstractItemView::item:disabled{
                color: rgba(0,0,0,90);
            }
        )");

        // 周一~周五：黑色
        QTextCharFormat weekdayFmt;
        weekdayFmt.setForeground(QBrush(QColor(0, 0, 0)));
        calendar->setWeekdayTextFormat(Qt::Monday,    weekdayFmt);
        calendar->setWeekdayTextFormat(Qt::Tuesday,   weekdayFmt);
        calendar->setWeekdayTextFormat(Qt::Wednesday, weekdayFmt);
        calendar->setWeekdayTextFormat(Qt::Thursday,  weekdayFmt);
        calendar->setWeekdayTextFormat(Qt::Friday,    weekdayFmt);

        // 周六/周日：红色
        QTextCharFormat weekendFmt;
        weekendFmt.setForeground(QBrush(QColor(220, 40, 40)));
        calendar->setWeekdayTextFormat(Qt::Saturday, weekendFmt);
        calendar->setWeekdayTextFormat(Qt::Sunday,   weekendFmt);
    }
}

bool TouchDateEdit::eventFilter(QObject *watched, QEvent *event)
{
    // 重入保护：避免“模拟点击”再次被 eventFilter 捕获导致递归崩溃
    if (m_inSynthClick) {
        return QDateEdit::eventFilter(watched, event);
    }

    if (watched == this && m_openOnClick && event->type() == QEvent::MouseButtonPress) {
        auto *me = static_cast<QMouseEvent*>(event);
        if (me->button() != Qt::LeftButton) {
            return QDateEdit::eventFilter(watched, event);
        }

        // 计算“下拉箭头按钮”的真实区域（跨平台更稳）
        QStyleOptionSpinBox opt;
        initStyleOption(&opt);
        QRect arrowRect = style()->subControlRect(
            QStyle::CC_SpinBox,
            &opt,
            QStyle::SC_SpinBoxDown,    // Qt 5 正确
            this
        );

        if (!arrowRect.isValid()) {
            // 如果拿不到，直接走默认流程，不强行处理
            return QDateEdit::eventFilter(watched, event);
        }

        QPoint clickPos = arrowRect.center();

        // 开始模拟点击（此时开启重入保护）
        m_inSynthClick = true;

        QMouseEvent press(QEvent::MouseButtonPress, clickPos,
                          Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        QApplication::sendEvent(this, &press);

        QMouseEvent release(QEvent::MouseButtonRelease, clickPos,
                            Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        QApplication::sendEvent(this, &release);

        m_inSynthClick = false;

        // 吃掉原始事件（避免同时触发光标/选择等）
        return true;
    }

    return QDateEdit::eventFilter(watched, event);
}
