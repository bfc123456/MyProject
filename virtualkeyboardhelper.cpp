// virtualkeyboardhelper.cpp
#include "virtualkeyboardhelper.h"
#include <QQuickWidget>
#include <QGuiApplication>
#include <QInputMethod>
#include <QEvent>
#include <QUrl>

static VirtualKeyboardHelper* s_helper = nullptr;

VirtualKeyboardHelper* VirtualKeyboardHelper::attachTo(QWidget* host,
                                                       const char* qmlUrl)
{
    if (s_helper) return s_helper;
    s_helper = new VirtualKeyboardHelper(host, qmlUrl);
    return s_helper;
}

VirtualKeyboardHelper::VirtualKeyboardHelper(QWidget* host, const char* qmlUrl)
    : QObject(host), m_host(host)
{
    // 1) 建立一个不进布局的透明覆盖层
    m_overlay = new QWidget(host);
    m_overlay->setAutoFillBackground(false);
    m_overlay->setAttribute(Qt::WA_NoSystemBackground);
    m_overlay->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    m_overlay->setGeometry(host->rect());
    m_overlay->hide();

    // 2) 在覆盖层里放 QQuickWidget，加载键盘 QML
    m_view = new QQuickWidget(m_overlay);
    m_view->setClearColor(Qt::transparent);
    m_view->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_view->setSource(QUrl(qmlUrl));
    m_view->setGeometry(m_overlay->rect());

    // 3) 跟随宿主窗口大小变化
    host->installEventFilter(this);

    // 4) 只在输入法可见时显示覆盖层
    auto im = qApp->inputMethod();
    connect(im, &QInputMethod::visibleChanged, this, [this]{
        const bool v = qApp->inputMethod()->isVisible();
        if (v) {
            m_overlay->show();
            m_overlay->raise();
            m_overlay->setAttribute(Qt::WA_TransparentForMouseEvents, false);
        } else {
            m_overlay->hide();
            m_overlay->setAttribute(Qt::WA_TransparentForMouseEvents, true);
        }
    });

    // 与当前可见状态同步一次
    if (im->isVisible()) {
        m_overlay->show();
        m_overlay->raise();
        m_overlay->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    }
}

bool VirtualKeyboardHelper::eventFilter(QObject* watched, QEvent* e)
{
    if (watched == m_host && (e->type() == QEvent::Resize || e->type() == QEvent::Show)) {
        if (m_overlay && m_view) {
            m_overlay->setGeometry(static_cast<QWidget*>(watched)->rect());
            m_view->setGeometry(m_overlay->rect());
        }
    }
    return QObject::eventFilter(watched, e);
}
