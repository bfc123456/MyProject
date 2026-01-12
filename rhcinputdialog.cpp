/******************************************************************************
 * 文件名：rhcinputdialog.cpp
 * 功能描述：实现RHC输入的对话框界面，允许用户输入RA、RV、PA、PCWP等多组参数。
 * 版本：v1.0.0
 * 作者：
 * 时间：2025-12-25
 * 版本控制：v1.0.0 - 初始版本，提供基本的RHC输入功能
 * 备注：该对话框用于在临床或研究场景下输入RHC检查结果的各项血流动力学参数，
 *       提供清除与保存功能，并支持自定义键盘输入，适配不同分辨率。
 ******************************************************************************/

#include "rhcinputdialog.h"

//1) Project Headers
#include "customkeyboard.h"
#include "closeonlywindow.h"

//2) Qt Headers
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QGuiApplication>
#include <QScreen>
#include <QGraphicsDropShadowEffect>

RHCInputDialog::RHCInputDialog(QWidget *pParent)
    : CloseOnlyWindow(pParent)
{
    // 获取屏幕分辨率
    QScreen *pScreen = QGuiApplication::primaryScreen();
    QRect rectScreenGeometry = pScreen->geometry();
    int nScreenWidth = rectScreenGeometry.width();
    int nScreenHeight = rectScreenGeometry.height();

    // 计算缩放比例
    m_fScaleX = (float)nScreenWidth / 1024;
    m_fScaleY = (float)nScreenHeight / 600;

    // 设置透明外层 + 无边框
    setFixedSize(int(420 * m_fScaleX), int(240 * m_fScaleY));
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::CustomizeWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    // 卡片主体
    QFrame *pCard = new QFrame(this);
    pCard->setObjectName("card");
    pCard->setFixedSize(size());
    pCard->setStyleSheet(R"(
        #card{
            background-color: #262A33;
            border: 1px solid rgba(255,255,255,90);
            border-radius: 16px;
        }
        #card QLabel{
            background: transparent;
            color: rgba(255,255,255,220);
        }
    )");

    QVBoxLayout *pMainLayout = new QVBoxLayout(pCard);
    pMainLayout->setContentsMargins(15 * m_fScaleX, 0 * m_fScaleY, 15 * m_fScaleX, 0 * m_fScaleY);

    // 标题栏：标题 + 关闭按钮（透明，无轮廓）
    QLabel *pTitleLabel = new QLabel(tr("心输出量"), pCard);
    pTitleLabel->setStyleSheet("font-size:18px;font-weight:800;color:rgba(255,225,225,230);");

    // 创建关闭按钮
    QPushButton *pCloseButton = new QPushButton(this);
    pCloseButton->setObjectName("closeBtn");
    pCloseButton->setIcon(QIcon(":/image/icons-close.png"));
    pCloseButton->setIconSize(QSize(20 * m_fScaleX, 20 * m_fScaleX));
    pCloseButton->setStyleSheet(R"(
        #closeBtn{
            background: transparent;
            border:none; border-radius: 15px;
            color: rgba(255,255,255,170);
            font-size: 14px; font-weight: 800;
        }
        #closeBtn:pressed{
            background : rgba(0,0,0,70);
        }
    )");

    // 创建按钮布局，用于设置关闭按钮的显示位置
    QHBoxLayout *pTopBarLayout = new QHBoxLayout();
    pTopBarLayout->setContentsMargins(0, 0, 0, 0);
    pTopBarLayout->addWidget(pTitleLabel);
    pTopBarLayout->addStretch();
    pTopBarLayout->addWidget(pCloseButton);

    // 创建网格布局
    QGridLayout *pGridLayout = new QGridLayout();

    // 创建输入框和分隔符
    m_pRaInput1Edit = new QLineEdit(pCard);
    m_pRvInput1Edit = new QLineEdit(pCard);
    m_pRvInput2Edit = new QLineEdit(pCard);
    QLabel *pRvDivider = new QLabel("/", pCard);

    m_pPaInput1Edit = new QLineEdit(pCard);
    m_pPaInput2Edit = new QLineEdit(pCard);
    m_pPaInput3Edit = new QLineEdit(pCard);
    QLabel *pPaDivider = new QLabel("/", pCard);

    m_pPcwpInput1Edit = new QLineEdit(pCard);

    // 定义QSS样式表
    QString strInputStyle = R"(
        QLineEdit {
            border: 1px solid #ccd1d9;
            border-radius: 8px;  /* 增加圆角 */
            padding: 8px 12px;
            background-color: #f7f7f7;  /* 浅灰色背景 */
            color: black;
            font-size: 16px;  /* 更大的字体 */
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);  /* 增加阴影 */
        }
        QLineEdit:focus {
            border: 1px solid #4CAF50;  /* 焦点时边框变绿 */
            background-color: #e8f5e9;  /* 焦点时背景变绿 */
        }
    )";

    // 应用QSS样式表到每个QLineEdit控件
    m_pRaInput1Edit->setStyleSheet(strInputStyle);
    m_pRvInput1Edit->setStyleSheet(strInputStyle);
    m_pRvInput2Edit->setStyleSheet(strInputStyle);
    m_pPaInput1Edit->setStyleSheet(strInputStyle);
    m_pPaInput2Edit->setStyleSheet(strInputStyle);
    m_pPaInput3Edit->setStyleSheet(strInputStyle);
    m_pPcwpInput1Edit->setStyleSheet(strInputStyle);

    // 设置焦点策略
    m_pRaInput1Edit->setFocusPolicy(Qt::ClickFocus);
    m_pRvInput1Edit->setFocusPolicy(Qt::ClickFocus);
    m_pRvInput2Edit->setFocusPolicy(Qt::ClickFocus);
    m_pPaInput1Edit->setFocusPolicy(Qt::ClickFocus);
    m_pPaInput2Edit->setFocusPolicy(Qt::ClickFocus);
    m_pPaInput3Edit->setFocusPolicy(Qt::ClickFocus);
    m_pPcwpInput1Edit->setFocusPolicy(Qt::ClickFocus);

    // 设置QLabel样式
    pRvDivider->setStyleSheet("QLabel { font-size: 18px; color: #aaaaaa; }");
    pPaDivider->setStyleSheet("QLabel { font-size: 18px; color: #aaaaaa; }");

    // 设置布局
    pGridLayout->addWidget(new QLabel("RA", pCard), 0, 0);
    pGridLayout->addWidget(m_pRaInput1Edit, 0, 4);

    pGridLayout->addWidget(new QLabel("RV", pCard), 1, 0);
    pGridLayout->addWidget(m_pRvInput1Edit, 1, 1);
    pGridLayout->addWidget(pRvDivider, 1, 2);
    pGridLayout->addWidget(m_pRvInput2Edit, 1, 3);

    pGridLayout->addWidget(new QLabel("PA", pCard), 2, 0);
    pGridLayout->addWidget(m_pPaInput1Edit, 2, 1);
    pGridLayout->addWidget(pPaDivider, 2, 2);
    pGridLayout->addWidget(m_pPaInput2Edit, 2, 3);
    pGridLayout->addWidget(m_pPaInput3Edit, 2, 4);

    pGridLayout->addWidget(new QLabel("PCWP", pCard), 3, 0);
    pGridLayout->addWidget(m_pPcwpInput1Edit, 3, 4);

    // 添加按钮
    QHBoxLayout *pBtnLayout = new QHBoxLayout();
    m_pClearBtn = new QPushButton(tr("全部清除"), pCard);
    m_pClearBtn->setIcon(QIcon(":/image/delete.png"));
    m_pClearBtn->setFixedSize(120 * m_fScaleX, 35 * m_fScaleY);
    m_pSaveBtn = new QPushButton(tr("保存"), pCard);
    m_pSaveBtn->setFixedSize(120 * m_fScaleX, 35 * m_fScaleY);
    m_pSaveBtn->setIcon(QIcon(":/image/icons8-save.png"));

    QString strButtonStyle = R"(
        QPushButton {
            background-color: #2196F3;
            border: none;
            border-radius: 6px;
            color: white;
            font-weight: bold;
            font-size: 14px;
            padding: 8px 20px;
        }
        QPushButton:pressed {
            background-color: #1976D2;
            padding-left: 2px;
            padding-top: 2px;
        }
    )";

    // Apply the same style to both buttons
    m_pClearBtn->setStyleSheet(strButtonStyle);
    m_pSaveBtn->setStyleSheet(strButtonStyle);

    pBtnLayout->addWidget(m_pClearBtn);
    pBtnLayout->addStretch();
    pBtnLayout->addWidget(m_pSaveBtn);

    pMainLayout->addLayout(pTopBarLayout);
    pMainLayout->addLayout(pGridLayout);
    pMainLayout->addLayout(pBtnLayout);

    // 连接信号槽
    connect(m_pClearBtn, &QPushButton::clicked, this, &RHCInputDialog::clearAllFields);
    connect(pCloseButton, &QPushButton::clicked, this, &RHCInputDialog::close);
}

RHCInputDialog::~RHCInputDialog()
{
    // 释放资源
}

QString RHCInputDialog::getRHCValue() const
{
    return m_pRhcInputEdit->text();  // 返回在输入框中输入的值
}

void RHCInputDialog::changeEvent(QEvent *pEvent)
{
    QWidget::changeEvent(pEvent);
    if (pEvent->type() == QEvent::LanguageChange) {
        setWindowTitle(tr("输入RHC"));
        m_pClearBtn->setText(tr("全部清除"));
        m_pSaveBtn->setText(tr("保存"));
    }
}

void RHCInputDialog::clearAllFields()
{
    m_pRaInput1Edit->clear();
    m_pRvInput1Edit->clear();
    m_pRvInput2Edit->clear();
    m_pPaInput1Edit->clear();
    m_pPaInput2Edit->clear();
    m_pPaInput3Edit->clear();
    m_pPcwpInput1Edit->clear();
}
