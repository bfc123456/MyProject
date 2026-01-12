/******************************************************************************
 * 文件名：cardiacoutputdialog.cpp
 * 功能描述：实现心输出量输入的对话框界面，允许用户输入心输出量（CO）。
 *           根据血压和平均值自动生成界面，并支持自定义数字键盘。
 * 版本：v1.0.0
 * 作者：
 * 时间：2025/12/25
 * 版本控制：v1.0.0 - 初始版本，提供基本的心输出量输入功能
 * 备注：该文件包含一个对话框，用于输入和显示心输出量。它通过继承CloseOnlyWindow，
 *       提供了一个只包含关闭按钮的对话框，并实现了屏幕分辨率适配。
 ******************************************************************************/

//1) Project Headers
#include "customkeyboard.h"
#include "closeonlywindow.h"
#include "cardiacoutputdialog.h"

//2) Qt Headers
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QGuiApplication>
#include <QScreen>
#include <QGraphicsDropShadowEffect>

//构造函数
CardiacOutputDialog::CardiacOutputDialog(QString strBpValue, QString strAvgValue, QWidget *pParent)
    : CloseOnlyWindow(pParent)
{
    // 获取屏幕分辨率
    QScreen *pScreen = QGuiApplication::primaryScreen();
    QRect rectScreenGeometry = pScreen->geometry();

    int iScreenWidth = rectScreenGeometry.width();  // 获取屏幕宽度
    int iScreenHeight = rectScreenGeometry.height();  // 获取屏幕高度

    // 计算界面缩放比例
    m_pScaleX = (float)iScreenWidth / 1024;  // X轴缩放比例
    m_pScaleY = (float)iScreenHeight / 600;  // Y轴缩放比例

    // 设置无边框透明背景的对话框
    setFixedSize(int(420 * m_pScaleX), int(240 * m_pScaleY));
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    // 初始化卡片主体
    QFrame *m_pCardFrame = new QFrame(this);
    m_pCardFrame->setObjectName("card");
    m_pCardFrame->setFixedSize(size());
    m_pCardFrame->setStyleSheet(R"(
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

    // 设置卡片内部布局
    QVBoxLayout *m_pMainLayout = new QVBoxLayout(m_pCardFrame);
    m_pMainLayout->setContentsMargins(22 * m_pScaleX, 18 * m_pScaleY, 22 * m_pScaleX, 18 * m_pScaleY);
    m_pMainLayout->setSpacing(int(14 * m_pScaleY));

    // 设置标题栏和关闭按钮
    m_pTitleLbl = new QLabel(tr("心输出量"), m_pCardFrame);
    m_pTitleLbl->setStyleSheet("font-size:18px;font-weight:800;color:rgba(255,225,225,230);");

    m_pCloseBtn = new QPushButton(this);
    m_pCloseBtn->setObjectName("closeBtn");
    m_pCloseBtn->setIcon(QIcon(":/image/icons-close.png"));
    m_pCloseBtn->setIconSize(QSize(20 * m_pScaleX, 20 * m_pScaleX));
    m_pCloseBtn->setStyleSheet(R"(
        #closeBtn{
            background: transparent;
            border:none;
            border-radius: 15px;
            color: rgba(255,255,255,170);
            font-size: 14px;
            font-weight: 800;
        }
        #closeBtn:pressed{
            background : rgba(0,0,0,70);
        }
    )");

    // 添加布局
    QHBoxLayout *m_pTopBar = new QHBoxLayout();
    m_pTopBar->setContentsMargins(0, 0, 0, 0);
    m_pTopBar->addWidget(m_pTitleLbl);
    m_pTopBar->addStretch();
    m_pTopBar->addWidget(m_pCloseBtn);
    m_pMainLayout->addLayout(m_pTopBar);

    // 显示血压值和平均值
    m_pBpLbl = new QLabel(strBpValue, m_pCardFrame);
    m_pAvgLbl = new QLabel(strAvgValue, m_pCardFrame);
    m_pUnitLbl = new QLabel("mmHg", m_pCardFrame);

    m_pBpLbl->setStyleSheet("font-size:28px; font-weight:800; color: white;");
    m_pAvgLbl->setStyleSheet("font-size:28px; font-weight:800; color: white;");
    m_pUnitLbl->setStyleSheet("font-size:12px; color: rgba(255,255,255,160);");

    QVBoxLayout *m_pDataUnitLayout = new QVBoxLayout();
    QHBoxLayout *m_pDataLayout = new QHBoxLayout();
    m_pDataLayout->addWidget(m_pBpLbl);
    m_pDataLayout->addSpacing(10 * m_pScaleX);
    m_pDataLayout->addWidget(m_pAvgLbl);
    m_pDataUnitLayout->addLayout(m_pDataLayout);
    m_pDataUnitLayout->addWidget(m_pUnitLbl);
    m_pMainLayout->addLayout(m_pDataUnitLayout);

    // 设置心输出量输入框
    m_pCoTitleLbl = new QLabel(tr("心输出量"), this);
    m_pCoTitleLbl->setStyleSheet("background-color: transparent; color: white; font-size: 16px;");
    m_pCoTitleLbl->setFixedHeight(40 * m_pScaleY);
    m_pCoEdit = new QLineEdit;
    m_pCoEdit->setFixedSize(135 * m_pScaleX, 35 * m_pScaleY);
    m_pCoEdit->setStyleSheet(R"(
                          QLineEdit {
                              border: 1px solid #ccd1d9;
                              border-radius: 8px;
                              padding: 8px 12px;
                              background-color: #f7f7f7;
                              color: black;
                              font-size: 16px;
                              box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
                          }
                          QLineEdit:focus {
                              border: 1px solid #4CAF50;
                              background-color: #e8f5e9;
                          }
                            )");

    m_pCoEdit->setFocusPolicy(Qt::ClickFocus);
    m_pCoUnitLbl = new QLabel("L/Min", this);
    m_pCoUnitLbl->setStyleSheet("background-color: transparent; color: white; font-size: 16px;");
    m_pCoUnitLbl->setFixedHeight(40 * m_pScaleY);

    QHBoxLayout *m_pCoLayout = new QHBoxLayout();
    m_pCoLayout->addStretch();
    m_pCoLayout->addWidget(m_pCoTitleLbl);
    m_pCoLayout->addSpacing(10 * m_pScaleX);
    m_pCoLayout->addWidget(m_pCoEdit);
    m_pCoLayout->addSpacing(5 * m_pScaleX);
    m_pCoLayout->addWidget(m_pCoUnitLbl);
    m_pCoLayout->addStretch();
    m_pMainLayout->addLayout(m_pCoLayout);

    // 添加按钮布局
    m_pBackBtn = new QPushButton(tr("返回"), this);
    m_pSaveBtn = new QPushButton(tr("保存"), this);
    m_pBackBtn->setFixedSize(120 * m_pScaleX, 45 * m_pScaleY);
    m_pSaveBtn->setFixedSize(120 * m_pScaleX, 45 * m_pScaleY);

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

    m_pBackBtn->setStyleSheet(strButtonStyle);
    m_pSaveBtn->setStyleSheet(strButtonStyle);

    QHBoxLayout *m_pBtnLayout = new QHBoxLayout();
    m_pBtnLayout->addWidget(m_pBackBtn);
    m_pBtnLayout->addStretch(1 * m_pScaleX);
    m_pBtnLayout->addWidget(m_pSaveBtn);
    m_pMainLayout->addLayout(m_pBtnLayout);

    // 连接按钮的信号槽
    connect(m_pBackBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_pSaveBtn, &QPushButton::clicked, this, &QDialog::accept);
    connect(m_pCloseBtn, &QPushButton::clicked, this, &CardiacOutputDialog::close);
}

//析构函数
CardiacOutputDialog::~CardiacOutputDialog() {
}

/***********************************************************************************************
 * FUNC    : GetCOValue
 * IN      : None
 * OUT     : None
 * RETURN  : QString - 心输出量（CO）输入框中的文本
 * AUTHOR  : 2025-12-29 Create by lxh for CardiacOutputDialog class
 * NOTE    : 获取当前界面中 CO 输入框的内容，通常用于保存/上传测量附加信息。
 ************************************************************************************************/
QString CardiacOutputDialog::GetCOValue() const {
    return m_pCoEdit->text();  // 返回用户输入的心输出量值
}
