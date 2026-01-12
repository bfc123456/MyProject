
/********************************************************************************/
/* 文件名    : closeonlywindow.cpp                                                */
/* 功能      : 消息对话框自定义界面（去除最小化与最大化，仅保留关闭按钮）               */
/* 版本      : 1.0.0                                                              */
/* 作者      :                                                         */
/* 日期      : 2025-12-26                                                       */
/* 说明      : 该文件实现了消息对话框界面并提供界面关闭功能   */
/********************************************************************************/

//1) Project Headers
#include "custommessagebox.h"

//2) Qt Headers
#include <QGuiApplication>
#include <QScreen>
#include <QFrame>
#include <QGraphicsDropShadowEffect>

//构造函数
CustomMessageBox::CustomMessageBox(QWidget *pParent, const QString &strTitle, const QString &strMessage,
                                   const QVector<QString> &vecStrButtons, int iWidth)
    : CloseOnlyWindow(pParent),
      strUserResponse("No")
{
    setWindowTitle(strTitle);
    // ========= 屏幕缩放 =========
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int screenWidth  = screenGeometry.width();
    int screenHeight = screenGeometry.height();

    m_fScaleX = (float)screenWidth  / 1024;
    m_fScaleY = (float)screenHeight / 600;

    // ========= 无边框 + 透明背景（让外面看起来是浮在主界面上的卡片） =========
    setWindowFlags(Qt::Dialog |
                   Qt::FramelessWindowHint |
                   Qt::CustomizeWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    // 外层不要设置背景色，让它保持透明
    // setStyleSheet(...) 改成只给内部 card 用

    // ========= 中间卡片容器 =========
    QFrame *cardFrame = new QFrame(this);
    cardFrame->setObjectName("msgCard");
    cardFrame->setFixedWidth(iWidth);                 // 传进来的宽度给卡片用
    cardFrame->setStyleSheet(R"(
        #msgCard {
            background-color: #262A33;          /* 雅雅黑深灰 */
            border-radius: 16px;
            border: 1px solid rgba(255,255,255,90);  /* 细浅白边 */
        }
        #msgCard QLabel {
            color: white;
            font-size: 14px;
        }
    )");

    // 阴影效果（让卡片“浮起来”）
    auto *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(28);
    shadow->setOffset(0, 10);
    shadow->setColor(QColor(0, 0, 0, 180));
    cardFrame->setGraphicsEffect(shadow);

    // ========= 标题栏：右上角关闭按钮 =========
    QPushButton *pCloseButton = new QPushButton(cardFrame);
    pCloseButton->setIcon(QIcon(":/image/icons-close.png"));
    pCloseButton->setIconSize(QSize(20 * m_fScaleX, 20 * m_fScaleX));
    pCloseButton->setFixedSize(28* m_fScaleX, 28* m_fScaleX);
    pCloseButton->setCursor(Qt::PointingHandCursor);
    pCloseButton->setStyleSheet(R"(
        QPushButton {
            background-color: transparent;
            border: none;
        }
        QPushButton:hover {
            background-color: #3A3F4A;
            border-radius: 15px;
        }
    )");

    QHBoxLayout *headerLayout = new QHBoxLayout();
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->addStretch();
    headerLayout->addWidget(pCloseButton, 0, Qt::AlignRight);

    // ========= 中间图标 =========
    m_pIconLabel = new QLabel(cardFrame);
    m_pIconLabel->setAlignment(Qt::AlignCenter);
    m_pIconLabel->setStyleSheet("background-color: transparent;");
    QPixmap iconPixmap(":/image/exclamation_mark1.png");   // 确认这里路径是 :/image/...
    m_pIconLabel->setPixmap(iconPixmap.scaled(52 * m_fScaleX, 52 * m_fScaleY,
                                           Qt::KeepAspectRatio,
                                           Qt::SmoothTransformation));

    // ========= 消息文本 =========
    m_pMessageLabel = new QLabel(strMessage, cardFrame);
    m_pMessageLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_pMessageLabel->setAlignment(Qt::AlignCenter);
    m_pMessageLabel->setWordWrap(true);
    m_pMessageLabel->setStyleSheet(R"(
        QLabel {
            background-color: transparent;
            color: rgba(255,255,255,220);
            font-size: 15px;
        }
    )");

    // ========= 底部按钮组 =========
    QHBoxLayout* pBtnLayout = new QHBoxLayout();
    pBtnLayout->setSpacing(36 * m_fScaleX);

    for (const QString &buttonText : vecStrButtons) {
        QPushButton *pBtn = new QPushButton(buttonText, cardFrame);
        pBtn->setFixedSize(120 * m_fScaleX, 40 * m_fScaleY);
        pBtn->setCursor(Qt::PointingHandCursor);

        pBtn->setStyleSheet(R"(
            QPushButton {
                background-color: #1E8CFF;      /* 主色蓝 */
                border: none;
                border-radius: 6px;
                color: white;
                font-weight: bold;
                font-size: 14px;
                padding: 6px 20px;
            }
            QPushButton:hover {
                background-color: #3C9DFF;
            }
            QPushButton:pressed {
                background-color: #1673D2;
            }
        )");

        connect(pBtn, &QPushButton::clicked, this, [this, buttonText]() {
            strUserResponse = buttonText;
            accept();
        });

        vecBtnList.append(pBtn);
        pBtnLayout->addWidget(pBtn, 0, Qt::AlignCenter);
    }

    // ========= 卡片内部布局 =========
    QVBoxLayout *pCardLayout = new QVBoxLayout(cardFrame);
    pCardLayout->setContentsMargins(26 * m_fScaleX, 18 * m_fScaleY,
                                   26 * m_fScaleX, 24 * m_fScaleY);
    pCardLayout->setSpacing(18 * m_fScaleY);

    pCardLayout->addLayout(headerLayout);
    pCardLayout->addSpacing(10 * m_fScaleY);
    pCardLayout->addWidget(m_pIconLabel, 0, Qt::AlignCenter);
    pCardLayout->addSpacing(8 * m_fScaleY);
    pCardLayout->addWidget(m_pMessageLabel);
    pCardLayout->addSpacing(18 * m_fScaleY);
    pCardLayout->addLayout(pBtnLayout);

    // ========= 外层布局：让卡片在整个对话框中居中 =========
    QVBoxLayout* pMainLayout = new QVBoxLayout(this);
    pMainLayout->setContentsMargins(0, 0, 0, 0);
    pMainLayout->addStretch();
    pMainLayout->addWidget(cardFrame, 0, Qt::AlignCenter);
    pMainLayout->addStretch();

    // 让窗口整体大小比卡片稍大一点（透明区域）
    int dialogW = iWidth + 80 * m_fScaleX;
    int dialogH = cardFrame->sizeHint().height() + 80 * m_fScaleY;
    resize(dialogW, dialogH);

    // 关闭按钮信号
    connect(pCloseButton, &QPushButton::clicked, this, &CustomMessageBox::close);
}

/********************************************************************************/
/* FUNC    : ~CustomMessageBox                                                   */
/* IN      : None                                                                */
/* OUT     : None                                                                */
/* RETURN  : void                                                                */
/* AUTHOR  : 2025-12-26 Create by lxh for CustomMessageBox class                 */
/* NOTE    : 析构函数，负责清理 CustomMessageBox 对象的动态分配资源           */
/*          - 删除按钮列表中的每个按钮对象                                     */
/*          - 释放布局和其他控件                                                 */
/********************************************************************************/

CustomMessageBox::~CustomMessageBox()
{
    // 清理按钮列表中的每个按钮对象
    for (QPushButton *button : vecBtnList) {
        delete button;  // 删除动态分配的按钮
    }

    // 释放布局和其他控件（如果有需要）
    delete m_pIconLabel;
    delete m_pMessageLabel;
    delete m_pButtonLayout;
}

/********************************************************************************/
/* FUNC    : GetUserResponse                                                    */
/* IN      : None                                                                */
/* OUT     : None                                                                */
/* RETURN  : QString                                                             */
/* AUTHOR  : 2025-12-26 Create by lxh for CustomMessageBox class                 */
/* NOTE    : 获取用户操作的响应（如“是”或“否”）                                 */
/********************************************************************************/

QString CustomMessageBox::GetUserResponse()
{
    return strUserResponse;
}
