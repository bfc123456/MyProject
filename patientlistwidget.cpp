#include "patientlistwidget.h"
#include "settingswidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QGroupBox>
#include <QStyle>
#include <QApplication>
#include <QDebug>
#include <QCloseEvent>


PatientListWidget::PatientListWidget(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle(tr("患者查询界面"));

    this->setObjectName("PatientListWidget");
    this->setStyleSheet(R"(
    QWidget#PatientListWidget {
        background-color: qlineargradient(
            x1: 0, y1: 1,
            x2: 1, y2: 0,
            stop: 0 rgba(6, 15, 30, 255),      /* 更暗靛蓝：左下 */
            stop: 0.5 rgba(18, 35, 65, 255),   /* 中段冷蓝 */
            stop: 1 rgba(30, 60, 100, 255)     /* 右上：深蓝灰 */
        );
    }
    )");

    this->resize(1024, 600);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);

    QWidget *topwidget = new QWidget(this);
    topwidget->setObjectName("TopBar");
    topwidget->setStyleSheet(R"(
        #TopBar {
            background-color: qlineargradient(
                x1: 0, y1: 0, x2: 0, y2: 1,
                stop: 0 rgba(25, 50, 75, 0.9),
                stop: 1 rgba(10, 20, 30, 0.85)
            );
            border-radius: 10px;
            border: 1px solid rgba(255, 255, 255, 0.08); /* 边缘高光 */
        }
    )");
    QBoxLayout *topLayout = new QHBoxLayout(topwidget);
    topwidget->setObjectName("TopWidget");
    topwidget->setFixedHeight(50);


    // 标题栏
    QLabel *title = new QLabel(tr("医疗设备管理系统---患者列表"));
    title->setObjectName("titleLabel");
    title->setAttribute(Qt::WA_TranslucentBackground);
    title->setFixedHeight(30);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("color: white;");

    //设置按钮（右侧）
    QPushButton *btnSettings = new QPushButton(this);
    btnSettings->setIcon(QIcon(":/image/icons8-shezhi.png"));
    connect(btnSettings, &QPushButton::clicked, this, &PatientListWidget::openSettingsWindow);
    btnSettings->setObjectName("SettingsButton");
    btnSettings->setIconSize(QSize(24, 24));
    btnSettings->setFlat(true);  // 去除按钮边框
    // 设置点击视觉反馈
    btnSettings->setStyleSheet(R"(
        QPushButton#SettingsButton {
            border: none;
            background-color: transparent;
            border-radius: 20px; /* 让 hover/pressed 效果是圆的 */
        }
        QPushButton:pressed {
            background-color: rgba(255, 255, 255, 0.2);
        }
    )");

    topLayout->addWidget(title, 0, Qt::AlignVCenter | Qt::AlignLeft);
    topLayout->addStretch();
    topLayout->addWidget(btnSettings, 0, Qt::AlignVCenter | Qt::AlignRight);
    mainLayout->addWidget(topwidget);

    // 中间卡片
    QGroupBox *group = new QGroupBox();
    QVBoxLayout *groupLayout = new QVBoxLayout(group);

    QHBoxLayout *topRow = new QHBoxLayout();

    searchEdit = new QLineEdit();
    searchEdit->setPlaceholderText(tr("🔍 搜索患者..."));
    searchEdit->setFixedSize(200,40);
    searchEdit->setStyleSheet(R"(
      QLineEdit{
          background-color: #32495d;  /* 输入框背景颜色 */
          border: 2px solid #3e5261;   /* 输入框边框颜色 */
          border-radius: 8px;          /* 圆角边框 */
          padding: 6px;                /* 内边距 */
          color: white;                /* 文字颜色 */
          font-size: 16px;             /* 字体大小 */
          font-weight: bold;           /* 字体加粗 */
      }

      QLineEdit#SearchEdit:focus {
          border: 2px solid #66bbee;   /* 获得焦点时的边框颜色 */
          background-color: #2c3e4f;   /* 获得焦点时的背景颜色 */
      }

      QLineEdit#SearchEdit::placeholder {
          color: #99aabb;              /* 占位符文字颜色 */
      }

    )");

    btnSearch = new QPushButton();
    btnSearch->setText(tr("搜索"));
    btnSearch->setIcon(QIcon(":/image/search.png"));
    btnSearch->setFixedSize(100,40);

    topRow->addWidget(searchEdit);
    topRow->addWidget(btnSearch);
    topRow->addStretch();

    groupLayout->addLayout(topRow);

    // 表格设置
    tableWidget = new QTableWidget(3, 3);
    tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    tableWidget->setStyleSheet(R"(
        QHeaderView::section {
            background-color: #2b4a80;
            color: white;
            font-weight: bold;
            padding: 6px;
            border: none;
        }
        QTableWidget {
            background-color: #102040;
            color: white;
            gridline-color: #405070;
            selection-background-color: #3E8EFF;
            selection-color: white;
            alternate-background-color: #1a2c4c;
            border: none;
        }
        QTableView QTableCornerButton::section {
            background: #2b4a80;
        }
    )");
    tableWidget->setAlternatingRowColors(true);
    tableWidget->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
    tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);


    tableWidget->setHorizontalHeaderLabels({tr("序列号"), tr("日期"), tr("备注")});
    tableWidget->horizontalHeader()->setStretchLastSection(true);
    tableWidget->verticalHeader()->setVisible(false);
    tableWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);//显示下拉条
    tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);

    tableWidget->setItem(0, 0, new QTableWidgetItem("P202401001"));
    tableWidget->setItem(0, 1, new QTableWidgetItem("2024-01-15"));
    tableWidget->setItem(1, 0, new QTableWidgetItem("P202401002"));
    tableWidget->setItem(1, 1, new QTableWidgetItem("2024-01-15"));
    tableWidget->setItem(2, 0, new QTableWidgetItem("P202401003"));
    tableWidget->setItem(2, 1, new QTableWidgetItem("2024-01-15"));

    groupLayout->addWidget(tableWidget);
    mainLayout->addWidget(group);

    // 底部按钮
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnBack = new QPushButton(tr("返回"));
    btnBack->setIcon(QIcon(":/image/icons8-return.png"));
    btnBack->setObjectName("BackButton");
    btnBack->setFixedSize(135,35);
    btnAdd = new QPushButton(tr("添加患者信息"));
    btnAdd->setObjectName("AddButton");
    btnAdd->setIcon(QIcon(":/image/icons8-add.png"));
    btnAdd->setFixedSize(135,35);

    //关联信号与槽
    connect(btnBack, &QPushButton::clicked, this, &PatientListWidget::patientReturnLogin);

    btnLayout->addWidget(btnBack);
    btnLayout->addStretch();
    btnLayout->addWidget(btnAdd);
    mainLayout->addLayout(btnLayout);
}

PatientListWidget::~PatientListWidget(){

}


void PatientListWidget::closeEvent(QCloseEvent *event) {
    // 确保关闭时删除键盘
    if (currentKeyboard) {
        currentKeyboard->deleteLater();  // 删除当前键盘实例
        currentKeyboard = nullptr;  // 设置为 nullptr，防止后续使用无效指针
    }

    // 调用父类的 closeEvent 来确保默认处理
    event->accept();  // 确保继续执行关闭事件
}

void PatientListWidget::showEvent(QShowEvent *event) {
    if (!eventFilterInstalled) {
        // 销毁之前的键盘实例
        if (currentKeyboard) {
            currentKeyboard->deleteLater();  // 删除键盘实例
            currentKeyboard = nullptr;
        }

        // 创建新的键盘实例
        currentKeyboard = new CustomKeyboard(this);

        // 安装事件过滤器
        this->searchEdit->installEventFilter(currentKeyboard);

        QPoint offset(50, 100);  // 根据实际需要设置偏移量
        currentKeyboard->attachTo(this->searchEdit, offset);  // 调整键盘位置


        eventFilterInstalled = true;  // 设置标志为已安装
    }

    QWidget::showEvent(event);
}

