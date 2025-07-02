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
#include <QSqlQuery>
#include "CustomMessageBox.h"
#include <QGraphicsBlurEffect>

PatientListWidget::PatientListWidget(QWidget *parent)
    : FramelessWindow(parent)
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
    title = new QLabel(tr("医疗设备管理系统---患者列表"));
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
          font-size: 14px;             /* 字体大小 */
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

    // 拿到单例键盘
    currentKeyboard = CustomKeyboard::instance(this);

    // 给每个 QLineEdit 注册一次偏移（如果默认偏移都一样，就写同一个 QPoint）
    currentKeyboard->registerEdit(searchEdit,QPoint(0, 0));

    btnSearch = new QPushButton();
    btnSearch->setText(tr("搜索"));
    btnSearch->setIcon(QIcon(":/image/search.png"));
    connect(btnSearch, &QPushButton::clicked, this, &PatientListWidget::onSearchClicked);
    btnSearch->setFixedSize(100,40);

    topRow->addWidget(searchEdit);
    topRow->addWidget(btnSearch);
    topRow->addStretch();

    groupLayout->addLayout(topRow);

    // 表格设置
    tableWidget = new QTableWidget(3, 3,this);
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

    //从数据库中查询数据
    QSqlQuery query("SELECT sensor_id, planting_date FROM sensor_info");

    int row = 0;  // 从第一行开始插入数据
    while (query.next()) {
        QString sensorId = query.value(0).toString();
        QString date = query.value(1).toString();
//        QString remark = query.value(2).toString();

        // 插入数据到 QTableWidget 的每一行
        tableWidget->setItem(row, 0, new QTableWidgetItem(sensorId));
        tableWidget->setItem(row, 1, new QTableWidgetItem(date));
//        tableWidget->setItem(row, 2, new QTableWidgetItem(remark));

        row++;  // 插入完一行后移动到下一行
    }

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

    btnBack->setStyleSheet(R"(
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
        padding: 2px 5px;
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

    btnAdd->setStyleSheet(R"(
    QPushButton {
        background-color: qlineargradient(
            stop: 0 rgba(110, 220, 145, 180),
            stop: 1 rgba(58, 170, 94, 180)
        );
        border: 1px solid rgba(168, 234, 195, 0.6);
        border-radius: 6px;
        color: white;
        font-weight: bold;
        font-size: 14px;
        padding: 2px 5px;
    }

    QPushButton:pressed {
        background-color: qlineargradient(
            stop: 0 rgba(44, 128, 73, 200),
            stop: 1 rgba(29, 102, 53, 200)
        );
        padding-left: 2px;
        padding-top: 2px;
    }
    )");


    //关联信号与槽
    connect(btnBack, &QPushButton::clicked, this, &PatientListWidget::patientReturnLogin);
    connect(btnAdd, &QPushButton::clicked, this, &PatientListWidget::patientReturnMonitor);

    btnLayout->addWidget(btnBack);
    btnLayout->addStretch();
    btnLayout->addWidget(btnAdd);
    mainLayout->addLayout(btnLayout);
}

PatientListWidget::~PatientListWidget(){

}

void PatientListWidget::onSearchClicked() {
    QString searchText = searchEdit->text().trimmed();

    if (searchText.isEmpty()) {

        //创建遮罩
        QWidget *overlay = new QWidget(this);
//        overlay->setGeometry(this->rect());
        overlay->setStyleSheet("background-color: rgba(0, 0, 0, 100);"); // 可调透明度
        overlay->setAttribute(Qt::WA_TransparentForMouseEvents, false); // 拦截事件
        overlay->show();
        overlay->raise();

        //添加模糊效果
        QGraphicsBlurEffect *blur = new QGraphicsBlurEffect;
        blur->setBlurRadius(20);  // 可调强度：20~40
        this->setGraphicsEffect(blur);
        //创建信息对话框
        CustomMessageBox dlg(this,tr("错误"),tr("查询失败，请再次检查输入"), { tr("确定") },350*scaleX);
        dlg.exec();

        // 清除遮罩和模糊
        this->setGraphicsEffect(nullptr);
        overlay->close();
        overlay->deleteLater();

        return;
    }

    // 过滤表格中的数据
    filterTableByKeyword(searchText);
}

void PatientListWidget::filterTableByKeyword(const QString &keyword) {
    bool rowFound = false;  // 标记是否找到匹配的行

    // 遍历所有行，查找是否有包含关键词的行
    for (int row = 0; row < tableWidget->rowCount(); ++row) {
        bool rowMatches = false;

        // 检查每一列，看看是否有列包含关键字
        for (int col = 0; col < tableWidget->columnCount(); ++col) {
            QTableWidgetItem *item = tableWidget->item(row, col);
            if (item && item->text().contains(keyword, Qt::CaseInsensitive)) {
                rowMatches = true;
                break;  // 找到匹配项，跳出当前列的循环
            }
        }

        // 根据是否匹配关键字来显示或隐藏行
        tableWidget->setRowHidden(row, !rowMatches);  // 隐藏不匹配的行
        if (rowMatches) rowFound = true;  // 如果找到匹配的行，设置 rowFound 为 true
    }

    // 如果没有找到任何匹配的行，显示提示信息
    if (!rowFound) {

        //创建遮罩
        QWidget *overlay = new QWidget(this);
//        overlay->setGeometry(this->rect());
        overlay->setStyleSheet("background-color: rgba(0, 0, 0, 100);"); // 可调透明度
        overlay->setAttribute(Qt::WA_TransparentForMouseEvents, false); // 拦截事件
        overlay->show();
        overlay->raise();

        //添加模糊效果
        QGraphicsBlurEffect *blur = new QGraphicsBlurEffect;
        blur->setBlurRadius(20);  // 可调强度：20~40
        this->setGraphicsEffect(blur);
        //创建信息对话框
        CustomMessageBox dlg(this,tr("提示"),tr("未找到匹配的记录"), { tr("确定") },350);
        dlg.exec();

        // 清除遮罩和模糊
        this->setGraphicsEffect(nullptr);
        overlay->close();
        overlay->deleteLater();

        // 取消所有隐藏的行
        for (int row = 0; row < tableWidget->rowCount(); ++row) {
            tableWidget->setRowHidden(row, false);  // 显示所有行
        }

        return;
    }
}

void PatientListWidget::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);
    if (event->type() == QEvent::LanguageChange) {
        // 系统自动发送的 LanguageChange
        setWindowTitle(tr("患者查询界面"));
        title->setText(tr("医疗设备管理系统---患者列表"));
        searchEdit->setText(tr("🔍 搜索患者..."));
        btnSearch->setText(tr("搜索"));
        tableWidget->setHorizontalHeaderLabels({
            tr("序列号"),
            tr("日期"),
            tr("备注")
        });
        btnBack->setText(tr("返回"));
        btnAdd->setText(tr("添加患者信息"));

    }
}

void PatientListWidget::patientReturnMonitor(){
    if(!implantMonitor){
        implantMonitor = std::make_unique<ImplantMonitor>();
        implantMonitor->setWindowFlags(
            implantMonitor->windowFlags()
            | Qt::WindowStaysOnTopHint      // 保证新窗永远在最上
        );
    }
    this->hide();
    this->close();
    implantMonitor->show();
    implantMonitor->raise();
}

//void PatientListWidget::showKeyboard() {
//    CustomKeyboard* keyboard = CustomKeyboard::instance(this);  // 获取虚拟键盘实例
//    keyboard->registerEdit(searchEdit, QPoint(0, 0));  // 注册 QLineEdit 和偏移量
//    keyboard->showKeyboard();  // 显示虚拟键盘
//}

//void PatientListWidget::closeEvent(QCloseEvent *event) {
//    CustomKeyboard* keyboard = CustomKeyboard::instance();
//    if (keyboard) {
//        keyboard->closeKeyboard();  // 隐藏虚拟键盘
//    }
//    QWidget::closeEvent(event);  // 调用父类的关闭事件
//}
