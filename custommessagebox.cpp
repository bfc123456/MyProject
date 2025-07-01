#include "CustomMessageBox.h"
#include <QGuiApplication>
#include <QScreen>

CustomMessageBox::CustomMessageBox(QWidget *parent, const QString &title, const QString &message,
                                   const QVector<QString> &buttons, int width)
    : CloseOnlyWindow(parent), userResponse("No")  // 默认用户选择“No”
{
    setWindowTitle(title);
    this->setObjectName("Custommessagebox");
    this->setStyleSheet(R"(
        QWidget#Custommessagebox {
        background-color: qlineargradient(
            x1: 0, y1: 0, x2: 0, y2: 1,
            stop: 0 rgba(15, 34, 67, 200),     /* 深蓝：顶部 */
            stop: 1 rgba(10, 25, 50, 180)      /* 更深蓝：底部 */
        );
        border-radius: 12px;
        }
    )");

    // 获取屏幕分辨率
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int screenWidth = screenGeometry.width();
    int screenHeight = screenGeometry.height();

    // 计算缩放比例
    scaleX = (float)screenWidth / 1024;
    scaleY = (float)screenHeight / 600;


    // 设置固定宽度，自动计算高度
    setFixedWidth(width);
    adjustSize();

    // 创建自定义图标标签
    iconLabel = new QLabel(this);
    iconLabel->setStyleSheet("font-weight: bold; font-"
                              "size: 16px; background-color: transparent; color: white;");
    QPixmap iconPixmap(":image/exclamation_mark.png");  // 使用固定的默认图标路径
    iconLabel->setPixmap(iconPixmap.scaled(40*scaleX, 40*scaleY, Qt::KeepAspectRatio));  // 设置图标大小
    iconLabel->setAlignment(Qt::AlignCenter);  // 将图标居中

    // 创建自定义消息标签
    messageLabel = new QLabel(message, this);
    messageLabel->setAlignment(Qt::AlignCenter);
    messageLabel->setWordWrap(true);
    messageLabel->setStyleSheet("font-weight: bold; font-" "size: 16px; background-color: transparent; color: white;");  // 确保字体变白

    //限制最大宽度，避免超出窗口
    messageLabel->setMaximumWidth(width - 40*scaleX);
    messageLabel->setMinimumWidth(width - 60*scaleX);

    // 创建按钮，并连接它们的信号
    buttonLayout = new QHBoxLayout();
    for (const QString &buttonText : buttons) {
        QPushButton *button = new QPushButton(buttonText, this);

        button->setFixedSize(120*scaleX, 40*scaleY);
        // 设置按钮样式（默认蓝色按钮 + 白色文字）
        button->setStyleSheet(R"(
        QPushButton {
            background-color: qlineargradient(
                x1:0, y1:0, x2:0, y2:1,
                stop:0 rgba(95, 169, 246, 180),
                stop:1 rgba(49, 122, 198, 180)
            );
            border: 1px solid rgba(163, 211, 255, 0.6); /* 半透明高光边框 */
            border-radius: 6px;
            color: white;
            font-weight: bold;
            font-size: 14px;
            padding: 8px 20px;
        }

        QPushButton:pressed {
            background-color: qlineargradient(
                stop: 0 rgba(47, 106, 158, 200),
                stop: 1 rgba(31, 78, 121, 200)
            );
            padding-left: 2px;
            padding-top: 2px;
        }
        )");

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
    mainLayout->addSpacing(20*scaleY);  // 设置间距
    mainLayout->addLayout(buttonLayout);  // 添加按钮
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);
}

QString CustomMessageBox::getUserResponse()
{
    return userResponse;
}
