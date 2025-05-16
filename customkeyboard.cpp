#include "customkeyboard.h"
#include <QLineEdit>
#include <QStackedWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScreen>
#include <QGuiApplication>
#include <QMouseEvent>
#include <QTimer>
#include <QApplication>

CustomKeyboard* CustomKeyboard::instance(QWidget *parent)
{
    static CustomKeyboard* instance = nullptr;  // 静态局部变量保证只创建一次
    static std::once_flag onceFlag;  // 确保线程安全

    std::call_once(onceFlag, [&]() {
        instance = new CustomKeyboard(parent);  // 只创建一次
    });

    return instance;
}

CustomKeyboard::CustomKeyboard(QWidget *parent)
    : QWidget(parent)
{
    setParent(parent);
    setFixedSize(470, 250);
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_ShowWithoutActivating);

    stackedWidget = new QStackedWidget(this);
    QWidget *numPage   = createNumKeyboard();
    QWidget *alphaPage = createAlphaKeyboard();
    stackedWidget->addWidget(numPage);
    stackedWidget->addWidget(alphaPage);
    stackedWidget->setCurrentIndex(1);

    QVBoxLayout *mainLay = new QVBoxLayout(this);
    mainLay->setContentsMargins(5,5,5,5);
    mainLay->addWidget(stackedWidget);
    setLayout(mainLay);

    // 默认居中显示
    QRect area;
    if (parentWidget()) {
        area = parentWidget()->geometry();
    } else {
        area = QGuiApplication::primaryScreen()->availableGeometry();
    }
    move(area.center() - QPoint(width()/2, height()/2));

    this->setWindowFlags(this->windowFlags() | Qt::WindowDoesNotAcceptFocus);   //设置虚拟键盘窗口属性，避免抢焦点
}

CustomKeyboard::~CustomKeyboard() {}

void CustomKeyboard::registerEdit(QLineEdit *edit, const QPoint &offset)
{
    if (!edit) return;
    editOffsetMap[edit] = offset;
    edit->installEventFilter(this);
}

bool CustomKeyboard::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::FocusIn) {
        if (auto *edit = qobject_cast<QLineEdit*>(watched)) {
            currentEdit = edit;
            QPoint offset = editOffsetMap.value(edit, QPoint(-35,0));
            attachTo(edit, offset);
        }
    }
    else if (event->type() == QEvent::FocusOut) {
        if (auto *edit = qobject_cast<QLineEdit*>(watched)) {
            if (edit == currentEdit) {
                hideWithDelay();
            }
        }
    }
    return QWidget::eventFilter(watched, event);
}

void CustomKeyboard::attachTo(QLineEdit *edit, const QPoint &offset)
{
    if (!edit) return;
    // 断开旧连接，再连接
    disconnect(this, &CustomKeyboard::keyPressed, nullptr, nullptr);
    connect(this, &CustomKeyboard::keyPressed, this, [this](const QString &key){
        if (!currentEdit) return;
        if (key == "Del") currentEdit->backspace();
        else if (key == "Hide") { hide(); currentEdit->clearFocus(); }
        else currentEdit->insert(key);
    });

    // 计算位置
    QPoint global = edit->mapToGlobal(edit->rect().bottomLeft());
    global += offset;
    QRect screen = QGuiApplication::primaryScreen()->availableGeometry();
    QSize kbSize = size();
    if (global.x() + kbSize.width() > screen.right())
        global.setX(screen.right() - kbSize.width() - 10);
    if (global.y() + kbSize.height() > screen.bottom())
        global.setY(screen.bottom() - kbSize.height() - 10);

    move(global);
    show();
    currentEdit->setFocus(Qt::OtherFocusReason);
    QTimer::singleShot(0, [=](){
        currentEdit->setFocus(Qt::OtherFocusReason);
        currentEdit->setCursorPosition(currentEdit->text().length());
    });
}

// ... 以下为键盘布局与切换函数实现 ...

QWidget* CustomKeyboard::createNumKeyboard()
{
    QWidget *page = new QWidget(this);
    QVBoxLayout *vlay = new QVBoxLayout(page);
    vlay->setSpacing(5);
    auto makeRow = [&](QStringList keys){
        QHBoxLayout *h = new QHBoxLayout;
        for (auto &k : keys) {
            QPushButton *b = new QPushButton(k, page);
            b->setFocusPolicy(Qt::NoFocus);
            connect(b, &QPushButton::clicked, [=](){ emit keyPressed(k); });
            h->addWidget(b);
        }
        return h;
    };
    vlay->addLayout(makeRow({"1","2","3"}));
    vlay->addLayout(makeRow({"4","5","6"}));
    vlay->addLayout(makeRow({"7","8","9"}));
    vlay->addLayout(makeRow({".","0","Del"}));
    // Space + 切换
    QHBoxLayout *last = new QHBoxLayout;
    QPushButton *space = new QPushButton("Space", page);
    space->setFocusPolicy(Qt::NoFocus);
    connect(space, &QPushButton::clicked, [=](){ emit keyPressed(" "); });
    last->addWidget(space);
    QPushButton *abc = new QPushButton("ABC", page);
    connect(abc, &QPushButton::clicked, this, &CustomKeyboard::switchToAlphaKeyboard);
    last->addWidget(abc);
    vlay->addLayout(last);
    return page;
}

QWidget* CustomKeyboard::createAlphaKeyboard()
{
    QWidget *page = new QWidget(this);
    QVBoxLayout *vlay = new QVBoxLayout(page);
    vlay->setSpacing(5);
    auto makeAlphaRow = [&](QStringList keys){
        QHBoxLayout *h = new QHBoxLayout;
        for (auto &k : keys) {
            QPushButton *b = new QPushButton(k, page);
            b->setFocusPolicy(Qt::NoFocus);
            letterButtons.append(b);
            connect(b, &QPushButton::clicked, [=](){ emit keyPressed(b->text()); });
            h->addWidget(b);
        }
        return h;
    };
    vlay->addLayout(makeAlphaRow({"q","w","e","r","t","y","u","i","o","p"}));
    vlay->addLayout(makeAlphaRow({"a","s","d","f","g","h","j","k","l"}));
    // z-m + Del
    QHBoxLayout *row3 = new QHBoxLayout;
    for (auto &k: QStringList{"z","x","c","v","b","n","m","Del"}){
        QPushButton *b = new QPushButton(k, page);
        b->setFocusPolicy(Qt::NoFocus);
        if (k == "Del") connect(b, &QPushButton::clicked, [=](){ emit keyPressed("Del"); });
        else { letterButtons.append(b); connect(b, &QPushButton::clicked, [=](){ emit keyPressed(b->text()); }); }
        row3->addWidget(b);
    }
    vlay->addLayout(row3);
    // Bottom row
    QHBoxLayout *bottom = new QHBoxLayout;
    QPushButton *shift = new QPushButton("Shift", page);
    shift->setFocusPolicy(Qt::NoFocus);
    connect(shift, &QPushButton::clicked, this, &CustomKeyboard::handleShiftClicked);
    bottom->addWidget(shift);
    QPushButton *space = new QPushButton("Space", page);
    space->setFocusPolicy(Qt::NoFocus);
    space->setMinimumWidth(180);
    connect(space, &QPushButton::clicked, [=](){ emit keyPressed(" "); });
    bottom->addWidget(space);
    QPushButton *num = new QPushButton("?123", page);
    num->setFocusPolicy(Qt::NoFocus);
    connect(num, &QPushButton::clicked, this, &CustomKeyboard::switchToNumKeyboard);
    bottom->addWidget(num);
    QPushButton *hide = new QPushButton("Hide", page);
    hide->setFocusPolicy(Qt::NoFocus);
    connect(hide, &QPushButton::clicked, [=](){ emit keyPressed("Hide"); });
    bottom->addWidget(hide);
    vlay->addLayout(bottom);
    return page;
}

void CustomKeyboard::switchToNumKeyboard()   { stackedWidget->setCurrentIndex(0); }
void CustomKeyboard::switchToAlphaKeyboard() { stackedWidget->setCurrentIndex(1); }

void CustomKeyboard::handleShiftClicked()
{
    isUpperCase = !isUpperCase;
    for (auto *b: letterButtons) {
        QString t = b->text();
        b->setText(isUpperCase ? t.toUpper() : t.toLower());
    }
}

// 鼠标拖拽事件
void CustomKeyboard::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        m_dragging = true;
        m_dragPosition = e->globalPos() - frameGeometry().topLeft();
        e->accept();
    }
}

void CustomKeyboard::mouseMoveEvent(QMouseEvent *e)
{
    if (m_dragging && (e->buttons() & Qt::LeftButton)) {
        move(e->globalPos() - m_dragPosition);
        e->accept();
    }
}

void CustomKeyboard::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        m_dragging = false;
        e->accept();
    }
}

// 显示虚拟键盘
void CustomKeyboard::showKeyboard() {
    this->show();
    this->raise();  // 确保它显示在前面
}

// 关闭虚拟键盘
void CustomKeyboard::closeKeyboard() {
    this->hide();  // 隐藏虚拟键盘
}

void CustomKeyboard::hideWithDelay()
{
    QTimer::singleShot(100, this, [this]() {
        QWidget *focusWidget = QApplication::focusWidget();
        if (!this->isAncestorOf(focusWidget) && focusWidget != currentEdit) {
            currentEdit = nullptr;
            this->hide();
        }
    });
}
