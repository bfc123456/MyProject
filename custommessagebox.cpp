#include "CustomMessageBox.h"

CustomMessageBox::CustomMessageBox(QWidget *parent, const QString &title, const QString &message,
                                   const QVector<QString> &buttons, int width)
    : QDialog(parent), userResponse("No")  // 默认用户选择“No”
{
    setWindowTitle(title);

    // 设置固定宽度，自动计算高度
    setFixedWidth(width);

    // 创建自定义图标标签
    iconLabel = new QLabel(this);
    QPixmap iconPixmap(":image/exclamation_mark.png");  // 使用固定的默认图标路径
    iconLabel->setPixmap(iconPixmap.scaled(40, 40, Qt::KeepAspectRatio));  // 设置图标大小
    iconLabel->setAlignment(Qt::AlignCenter);  // 将图标居中

    // 创建自定义消息标签
    messageLabel = new QLabel(message, this);
    messageLabel->setAlignment(Qt::AlignCenter);
    messageLabel->setStyleSheet("color: white; font-size: 12px;");  // 确保字体变白

    // 创建按钮，并连接它们的信号
    buttonLayout = new QHBoxLayout();
    for (const QString &buttonText : buttons) {
        QPushButton *button = new QPushButton(buttonText, this);

        // 设置按钮样式（默认蓝色按钮 + 白色文字）
            button->setStyleSheet(
                "QPushButton {"
                "   background-color: #007BFF;"  // 蓝色背景
                "   color: white;"               // 文字白色
                "   font-size: 16px;"            // 字体大小
                "   font-weight: bold;"          // 字体加粗
                "   border-radius: 6px;"         // 圆角
                "   padding: 8px 16px;"          // 增加按钮内边距
                "   border: 2px solid #0056b3;"  // 深蓝色边框，增强对比度
                "}"
                "QPushButton:hover { background-color: #0056b3; }"
                "QPushButton:pressed { background-color: #003f7f; }"
            );

            // 悬停时变亮
            button->setStyleSheet(button->styleSheet() +
                "QPushButton:hover { background-color: #0056b3; }"
                "QPushButton:pressed { background-color: #003f7f; }");

        connect(button, &QPushButton::clicked, this, [this, buttonText](){
            userResponse = buttonText;  // 获取按钮文本作为响应
            accept();  // 关闭窗口
        });
        buttonList.append(button);
        buttonLayout->addWidget(button);
    }

    // 创建垂直布局
    mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(iconLabel);  // 添加图标
    mainLayout->addWidget(messageLabel);  // 添加消息
    mainLayout->addSpacing(20);  // 设置间距
    mainLayout->addLayout(buttonLayout);  // 添加按钮

    setLayout(mainLayout);  // 设置布局
}

QString CustomMessageBox::getUserResponse()
{
    return userResponse;
}
