#include "bluroverlayguard.h"
#include <QGraphicsBlurEffect>

BlurOverlayGuard::BlurOverlayGuard(QWidget *pHost,
                                   int iBlurRadius,
                                   int iAlpha)
    : m_pHostWidget(pHost)
{
    if (!m_pHostWidget)
        return;

    // 1) 创建半透明遮罩层
    m_pOverlayWidget = new QWidget(m_pHostWidget);
    m_pOverlayWidget->setGeometry(m_pHostWidget->rect());
    m_pOverlayWidget->setStyleSheet(
        QStringLiteral("background-color: rgba(0, 0, 0, %1);").arg(iAlpha)
    );
    // 拦截事件，防止点到底下去
    m_pOverlayWidget->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    m_pOverlayWidget->show();
    m_pOverlayWidget->raise();

    // 2) 添加模糊效果（让 Qt 自己管理生命周期）
    m_pBlurEffect = new QGraphicsBlurEffect(m_pHostWidget); // parent = m_pHostWidget
    m_pBlurEffect->setBlurRadius(iBlurRadius);
    m_pHostWidget->setGraphicsEffect(m_pBlurEffect);
}

BlurOverlayGuard::~BlurOverlayGuard()
{
    if (m_pHostWidget) {
        // 解除效果（Qt 会自动 delete m_pBlurEffect，因为 parent=m_pHostWidget）
        m_pHostWidget->setGraphicsEffect(nullptr);
    }

    // 遮罩层我们手动销毁，避免一直占着界面
    if (m_pOverlayWidget) {
        m_pOverlayWidget->hide();
        m_pOverlayWidget->deleteLater(); // 安全交给 Qt 事件循环删除
        m_pOverlayWidget = nullptr;
    }

    // 不需要 delete m_pBlurEffect（Qt 会处理）
    m_pBlurEffect = nullptr;
}
