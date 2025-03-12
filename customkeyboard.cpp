#include "customkeyboard.h"
#include <QPushButton>
#include <QStackedWidget>
#include <QSignalMapper>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>

CustomKeyboard::CustomKeyboard(QWidget *parent)
    : QWidget(parent)
{
    // 用 QStackedWidget 来装两个不同的键盘布局
    stackedWidget = new QStackedWidget(this);

    // 创建两个布局页面
    QWidget *numWidget   = createNumKeyboard();
    QWidget *alphaWidget = createAlphaKeyboard();

    // 加到stackedWidget
    stackedWidget->addWidget(numWidget);   // index 0
    stackedWidget->addWidget(alphaWidget); // index 1

    // 默认显示字母键盘（index 1），也可根据需求
    stackedWidget->setCurrentIndex(1);

    // 总体布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(stackedWidget);
    setLayout(mainLayout);

    // 你也可以加一个 QSS 让其更美观
    setStyleSheet(R"(
                  QWidget {
                  background-color: #f0f0f0;
                  }
                  QPushButton {
                  background-color: #ffffff;
                  color: #333333;
                  border: 1px solid #dddddd;
                  border-radius: 4px;
                  font-size: 16px;
                  min-width: 40px;
                  min-height: 40px;
                  }
                  QPushButton:pressed {
                  background-color: #cccccc;
                  }
                  )");
}

QWidget* CustomKeyboard::createNumKeyboard()
{
    // 数字键盘：几行数字 + Space + Del + 切换按钮
    QWidget *page = new QWidget(this);
    QVBoxLayout *vlay = new QVBoxLayout(page);
    vlay->setSpacing(5);

    // 行1: 1 2 3
    {
        QHBoxLayout *row = new QHBoxLayout;
        QStringList keys = {"1","2","3"};
        for (auto &k : keys) {
            QPushButton *btn = new QPushButton(k, page);

            // 设置按钮不抢焦点
            btn->setFocusPolicy(Qt::NoFocus);

            connect(btn, &QPushButton::clicked, [=](){
                emit keyPressed(k);
            });
            row->addWidget(btn);
        }
        vlay->addLayout(row);
    }

    // 行2: 4 5 6
    {
        QHBoxLayout *row = new QHBoxLayout;
        QStringList keys = {"4","5","6"};
        for (auto &k : keys) {
            QPushButton *btn = new QPushButton(k, page);
            btn->setFocusPolicy(Qt::NoFocus);
            connect(btn, &QPushButton::clicked, [=](){
                emit keyPressed(k);
            });
            row->addWidget(btn);
        }
        vlay->addLayout(row);
    }

    // 行3: 7 8 9
    {
        QHBoxLayout *row = new QHBoxLayout;
        QStringList keys = {"7","8","9"};
        for (auto &k : keys) {
            QPushButton *btn = new QPushButton(k, page);
            btn->setFocusPolicy(Qt::NoFocus);
            connect(btn, &QPushButton::clicked, [=](){
                emit keyPressed(k);
            });
            row->addWidget(btn);
        }
        vlay->addLayout(row);
    }

    // 行4: . 0 Del
    {
        QHBoxLayout *row = new QHBoxLayout;
        QStringList keys = {".","0","Del"};
        for (auto &k : keys) {
            QPushButton *btn = new QPushButton(k, page);
            btn->setFocusPolicy(Qt::NoFocus);
            // 如果是 Del, ...
            connect(btn, &QPushButton::clicked, [=](){
                emit keyPressed(k);
            });
            row->addWidget(btn);
        }
        vlay->addLayout(row);
    }

    // 行5: Space + 切换到字母(ABC)
    {
        QHBoxLayout *row = new QHBoxLayout;
        // Space
        QPushButton *spaceBtn = new QPushButton("Space", page);
        spaceBtn->setFocusPolicy(Qt::NoFocus);
        connect(spaceBtn, &QPushButton::clicked, [=](){
            emit keyPressed(" ");
        });
        row->addWidget(spaceBtn);

        // 切换按钮 => ABC
        QPushButton *abcBtn = new QPushButton("ABC", page);
        connect(abcBtn, &QPushButton::clicked, this, &CustomKeyboard::switchToAlphaKeyboard);
        row->addWidget(abcBtn);

        vlay->addLayout(row);
    }

    return page;
}

QWidget* CustomKeyboard::createAlphaKeyboard()
{
    QWidget *page = new QWidget(this);
    QVBoxLayout *vlay = new QVBoxLayout(page);
    vlay->setSpacing(5);

    // 行1: Q W E R T Y U I O P
    {
        QHBoxLayout *row = new QHBoxLayout;
        QStringList keys = {"q","w","e","r","t","y","u","i","o","p"};
        for (const QString &k : keys) {
            QPushButton *btn = new QPushButton(k, page);
            btn->setFocusPolicy(Qt::NoFocus);

            // 将此按钮保存到 letterButtons
            letterButtons.append(btn);

            connect(btn, &QPushButton::clicked, [=](){
                emit keyPressed(btn->text());
                // 注意，这里要发射当前文字（可能是大小写变化后的）
            });
            row->addWidget(btn);
        }
        vlay->addLayout(row);
    }

    // 行2: A S D F G H J K L
    {
        QHBoxLayout *row = new QHBoxLayout;
        QStringList keys = {"a","s","d","f","g","h","j","k","l"};
        for (const QString &k : keys) {
            QPushButton *btn = new QPushButton(k, page);
            btn->setFocusPolicy(Qt::NoFocus);

            // 保存到 letterButtons
            letterButtons.append(btn);

            connect(btn, &QPushButton::clicked, [=](){
                emit keyPressed(btn->text());
            });
            row->addWidget(btn);
        }
        vlay->addLayout(row);
    }

    // 第3行: Z X C V B N M Del
    {
        QHBoxLayout *row = new QHBoxLayout;
        QStringList keys = {"z","x","c","v","b","n","m","Del"};
        for (const QString &k : keys) {
            QPushButton *btn = new QPushButton(k, page);
            btn->setFocusPolicy(Qt::NoFocus);

            // 如果是 Del，就发射 "Del"
            if (k == "Del") {
                connect(btn, &QPushButton::clicked, [=](){
                    emit keyPressed("Del");
                });
            } else {
                // 如果是字母，加入 letterButtons 并发射当前文字
                letterButtons.append(btn);
                connect(btn, &QPushButton::clicked, [=](){
                    emit keyPressed(btn->text());
                });
            }
            row->addWidget(btn);
        }
        vlay->addLayout(row);
    }


    // 第4行: Shift, Space(大), ?123, Hide
    {
        QHBoxLayout *row = new QHBoxLayout;

        // Shift
        QPushButton *shiftBtn = new QPushButton("Shift", page);
        shiftBtn->setFocusPolicy(Qt::NoFocus);

        // 大小写切换逻辑省略
        connect(shiftBtn, &QPushButton::clicked, this, &CustomKeyboard::handleShiftClicked);
        row->addWidget(shiftBtn);

        // 给 Space 一个更大的最小宽度，或者用伸缩因子
        QPushButton *spaceBtn = new QPushButton("Space", page);
        spaceBtn->setFocusPolicy(Qt::NoFocus);

        spaceBtn->setMinimumWidth(180);

        connect(spaceBtn, &QPushButton::clicked, [=](){
            emit keyPressed(" ");
        });

        row->addWidget(spaceBtn);

        // ?123 切换按钮
        QPushButton *numBtn = new QPushButton("?123", page);
        numBtn->setFocusPolicy(Qt::NoFocus);
        connect(numBtn, &QPushButton::clicked, this, &CustomKeyboard::switchToNumKeyboard);
        row->addWidget(numBtn);

        // Hide
        QPushButton *hideBtn = new QPushButton("Hide", page);
        hideBtn->setFocusPolicy(Qt::NoFocus);

        hideBtn->setIcon(QIcon(":/image/hide.png"));
        hideBtn->setIconSize(QSize(24, 24));

        // 点击隐藏键盘
        connect(hideBtn, &QPushButton::clicked, [=](){
            this->setVisible(false);
        });
        row->addWidget(hideBtn);

        vlay->addLayout(row);
    }


    return page;
}


void CustomKeyboard::handleButtonClicked(const QString &text)
{
    // 也可以不用SignalMapper, 用lambda替代
    qDebug() << "Key pressed:" << text;
    emit keyPressed(text);
}

void CustomKeyboard::switchToNumKeyboard()
{
    stackedWidget->setCurrentIndex(0); // 显示数字键盘页面
}

void CustomKeyboard::switchToAlphaKeyboard()
{
    stackedWidget->setCurrentIndex(1); // 显示字母键盘页面
}

void CustomKeyboard::handleShiftClicked()
{
    // 翻转大小写模式
    isUpperCase = !isUpperCase;

    // 遍历所有字母按钮
    for (QPushButton *btn : letterButtons) {
        QString oldText = btn->text();
        if (isUpperCase) {
            btn->setText(oldText.toUpper());
        } else {
            btn->setText(oldText.toLower());
        }
    }

    qDebug() << "Shift clicked, isUpperCase=" << isUpperCase;
}



