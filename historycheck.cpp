#include "historycheck.h"
#include "ui_historycheck.h"
#include <QScrollArea>
#include <QVBoxLayout>
#include <QTableWidgetItem>
#include <QSqlError>
#include <QLineEdit>
#include <QIcon>
#include <QHeaderView>
#include <QMessageBox>
#include <QScreen>
#include "databasemanager.h"
#include "CustomMessageBox.h"
#include "customkeyboard.h"

HistoryCheck::HistoryCheck(databasemanager *db, QWidget *parent) :
    QMainWindow(parent),   // 改为继承 QMainWindow
    ui(new Ui::HistoryCheck),
    historyTable(nullptr),
    dbManager(db)   // 创建数据库管理对象
{
    // 初始化UI
    ui->setupUi(this);

    // 获取屏幕的大小
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();

    // 获取窗口的大小
    QSize dialogSize = this->size();

    // 计算窗口的中央位置
    int x = (screenGeometry.width() - dialogSize.width()) / 2;
    int y = (screenGeometry.height() - dialogSize.height()) / 2;

    // 移动窗口到屏幕中央
    this->move(x, y);

    this->setWindowTitle(tr("历史查询界面"));

    // 设置窗口整体样式
    this->setStyleSheet("background-color: #222222; color: white;");

    // 创建标签
    historylabel = new QLabel(tr("可查询记录"), this);  // 创建标签并设置内容
    historylabel->setAlignment(Qt::AlignCenter);  // 设置标签内容居中

    historylabel->setStyleSheet("font-size: 22px; color: white; font-weight: bold; font-family: 'Microsoft YaHei';");

    // 创建表格 + 滚动区域
    historyTable = new QTableWidget(this);
    historyTable->setColumnCount(6);  // 设置列数为6
    historyTable->setHorizontalHeaderLabels(QStringList() << "NO." << tr("舒张压(mmHg)") << tr("收缩压(mmHg)")<< tr("平均值(mmHg)") << tr("心率(次/分钟)") << tr("上传时间"));
    historyTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // **表头样式**
    historyTable->horizontalHeader()->setStyleSheet("QHeaderView::section { background-color:  #444444; font-weight: bold;  color: white; padding: 5px; border: 1px solid gray; }");

    // **表格整体样式**
    historyTable->setStyleSheet(
        "QTableWidget { font-size: 14px; border: 1px solid #555555; gridline-color: gray; background-color: #181818; color: white;}"
        "QTableWidget::item:selected { background-color: #555555; color: yellow; }"  // 选中时高亮
    );

    // **修改左上角交叠区域的颜色**
    historyTable->setStyleSheet(
        "QTableWidget { background-color: black; color: white; gridline-color: gray; }"
        "QTableCornerButton::section { background-color: #333333; border: 1px solid #555555; }"
        "QHeaderView::section { background-color: #444444; color: white; font-weight: bold; border: 1px solid #555555; }"
    );

    // 设置表头行高
    historyTable->horizontalHeader()->setDefaultSectionSize(100); // 设置表头行高
    historyTable->setRowHeight(1, 80);

    // **显示左侧行号**
    historyTable->verticalHeader()->setVisible(true); // 确保行号可见
    historyTable->verticalHeader()->setStyleSheet(
        "QHeaderView::section { background-color: #333333; color: white; font-weight: bold; padding: 5px; border: 1px solid #555555; }"
    );

    // 将表格放入一个 QScrollArea，以便有滚动条
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);  // 使得 QTableWidget 可以自适应大小
    scrollArea->setWidget(historyTable);  // 将表格放入滚动区域

    // 创建布局并添加滚动区域
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(historylabel);  // 将标签添加到布局顶部
    layout->addWidget(scrollArea);  // 添加滚动区域

    // 获取所有行数据，增加对“舒张压”行的样式设置
    for (int row = 0; row < historyTable->rowCount(); ++row) {
        // 获取“舒张压”列（第2列）的单元格
        QTableWidgetItem *diastolicItem = historyTable->item(row, 1); // 第2列是“舒张压”

        if (diastolicItem) {
            // 这里使用条件判断来加深背景颜色，且不改变其他行
            // 你可以修改此处的条件来满足你的需求
            if (row == 0) { // 假设要加样式的是第1行（舒张压所在行）
                // 设置背景颜色为深灰色
                for (int col = 0; col < historyTable->columnCount(); ++col) {
                    QTableWidgetItem *item = historyTable->item(row, col);
                    if (item) {
                        item->setBackgroundColor(QColor(169, 169, 169));  // 深灰色背景
                        // 设置字体加粗
                        QFont font = item->font();
                        font.setBold(true);
                        item->setFont(font);
                    }
                }
                // 设置该行的高度
                historyTable->setRowHeight(row, 80);  // 增大行高
            }
        }
    }

    // 创建底部控件，包含文本框和按钮
    QWidget *bottomWidget = new QWidget(this);
    QHBoxLayout *bottomLayout = new QHBoxLayout(bottomWidget);

    // 调整整体布局的内部间距
    bottomLayout->setSpacing(15); // 所有控件之间的间距

    // 创建输入框
    searchLineEdit = new QLineEdit(bottomWidget);  // 创建输入框
    searchLineEdit->setPlaceholderText(tr("输入关键字..."));  // 设置占位文本
    searchLineEdit->setMinimumWidth(180);
    searchLineEdit->installEventFilter(this);  // 安装事件过滤器

    // 创建搜索按钮
    searchButton = new QPushButton(tr("搜索"), bottomWidget);
    searchButton->setFixedSize(120, 40);
    searchButton->setIcon(QIcon(":/image/search.png"));  // 设置图标
    searchButton->setIconSize(QSize(24, 24));  // 设置图标的大小
    // 设置字体为黑体
    searchButton->setStyleSheet("background-color: #616161; color: white; padding: 10px 20px; border-radius: 10px;");

    connect(searchButton, &QPushButton::clicked, this, &HistoryCheck::searchData);  // 搜索按钮点击事件

    bottomLayout->addStretch(1);

    // 创建删除按钮
    deleteButton = new QPushButton(tr("删除"), bottomWidget);
    deleteButton->setFixedSize(120, 40);
    deleteButton->setIcon(QIcon(":/image/delete.png"));  // 设置图标
    deleteButton->setIconSize(QSize(24, 24));  // 设置图标的大小
    // 设置字体为黑体
    deleteButton->setStyleSheet("background-color: #D32F2F; color: white; padding: 10px 20px; border-radius: 10px;");

    connect(deleteButton, &QPushButton::clicked, this, &HistoryCheck::deleteData);  // 删除按钮点击事件

    bottomLayout->addWidget(searchLineEdit);
    bottomLayout->addWidget(searchButton);
    bottomLayout->addStretch(1); // 拉伸间隔
    bottomLayout->addWidget(deleteButton);

    // ============ 将上部内容打包到 topContainer ============

    QWidget *topContainer = new QWidget(this);
    QVBoxLayout *topLayout = new QVBoxLayout(topContainer);
    topLayout->addWidget(historylabel);
    topLayout->addWidget(scrollArea);
    topLayout->addWidget(bottomWidget);
    topLayout->setSpacing(10);
    topLayout->setContentsMargins(5,5,5,5);

    // ============ 自定义虚拟键盘 (底部) ============

    customKeyboard = new CustomKeyboard(this);
    customKeyboard->setFixedSize(900, 250);  // 限制键盘宽度和高度
    customKeyboard->setVisible(false);

    // 当按键被点击时，触发 onKeyboardKeyPressed
    connect(customKeyboard, &CustomKeyboard::keyPressed,
            this, &HistoryCheck::onKeyboardKeyPressed);

    connect(customKeyboard, &CustomKeyboard::hideKeyboardRequested,
            this, [=](){
                customKeyboard->setVisible(false);
            });

    // ============ 总体布局 ============

    // 用一个新的垂直布局管理 topContainer(2份) + customKeyboard(1份)
    QVBoxLayout *mainLayout = new QVBoxLayout;
    // 让 topContainer 占 2/3
    mainLayout->addWidget(topContainer, /*stretch=*/2);
    // 让键盘占 1/3
    mainLayout->addWidget(customKeyboard, /*stretch=*/1);

    mainLayout->setAlignment(customKeyboard, Qt::AlignHCenter);

    // 设为中央控件
    QWidget *central = new QWidget(this);
    central->setLayout(mainLayout);
    setCentralWidget(central);

    // ============ 后续逻辑 ============

    loadHistoryData(); // 加载数据
    historyTable->resizeColumnsToContents();
    historyTable->horizontalHeader()->setStretchLastSection(true);
}

HistoryCheck::~HistoryCheck()
{
    delete ui;
}

void HistoryCheck::loadHistoryData() {
    if (!dbManager->getDatabase().isOpen()) {
        qDebug() << "Database is not open!";
        return;  // 如果数据库未打开，直接返回
    } else {
        qDebug() << "Database connected successfully!";
    }

    QSqlQuery countQuery;
    countQuery.exec("SELECT COUNT(*) FROM pulmonary_artery_pressure");
    if (countQuery.next()) {
        int count = countQuery.value(0).toInt();
        qDebug() << "Number of rows in the table: " << count;

        if (count == 0) {
            qDebug() << "No data in the table.";
            return;  // 如果表中没有数据，直接返回
        }
    } else {
        qDebug() << "Failed to get row count: " << countQuery.lastError().text();
        return;  // 如果查询失败，直接返回
    }

    QSqlQuery query = dbManager->queryData();
    qDebug() << "Query executed successfully.";

    if (historyTable == nullptr) {
        qDebug() << "historyTable is not initialized properly!";
        return;
    }

    //     清空表格数据
    historyTable->clearContents();
    historyTable->setRowCount(0);  // 确保表格重置行数
    QCoreApplication::processEvents(); // 处理UI线程中的所有事件


    // 确保表格重置后再进行数据插入
    qDebug() << "Table reset, preparing to insert data.";

    int row = 0;
    while (query.next()) {
        // 获取每一列的数据
        int number = query.value("number").toInt();
        double diastolic = query.value("diastolicValue").toDouble();
        double systolic = query.value("systolicValue").toDouble();
        double avg = query.value("avgValue").toDouble();
        int heartbeat = query.value("heartbeatValue").toInt();
        QString uploadTime = query.value("upload_time").toString();

        // 打印每一行的数据
        qDebug() << "Row data: "
                 << "Number: " << number
                 << " Diastolic: " << diastolic
                 << " Systolic: " << systolic
                 << " Avg: " << avg
                 << " Heartbeat: " << heartbeat
                 << " Upload Time: " << uploadTime;

        // 插入数据到表格
        historyTable->insertRow(row);
        historyTable->setItem(row, 0, new QTableWidgetItem(QString::number(number)));
        historyTable->setItem(row, 1, new QTableWidgetItem(QString::number(diastolic)));
        historyTable->setItem(row, 2, new QTableWidgetItem(QString::number(systolic)));
        historyTable->setItem(row, 3, new QTableWidgetItem(QString::number(avg)));
        historyTable->setItem(row, 4, new QTableWidgetItem(QString::number(heartbeat)));
        historyTable->setItem(row, 5, new QTableWidgetItem(uploadTime));

        // 设置所有插入单元格的文本居中对齐
        historyTable->item(row, 0)->setTextAlignment(Qt::AlignCenter);
        historyTable->item(row, 1)->setTextAlignment(Qt::AlignCenter);
        historyTable->item(row, 2)->setTextAlignment(Qt::AlignCenter);
        historyTable->item(row, 3)->setTextAlignment(Qt::AlignCenter);
        historyTable->item(row, 4)->setTextAlignment(Qt::AlignCenter);
        historyTable->item(row, 5)->setTextAlignment(Qt::AlignCenter);

        historyTable->setColumnHidden(0, true);  // 隐藏第0列（number列）

        row++;
    }
    query.finish(); //释放结果集
}

// 搜索数据函数
void HistoryCheck::searchData() {
//    qDebug()<<"serchData has colcked";
    QString keyword = searchLineEdit->text();  // 获取用户输入的关键词
//    qDebug()<<"serchData has get text";

    if (keyword.isEmpty()) {
        // 如果没有输入关键词，显示所有数据
        loadHistoryData();  // 调用加载所有数据的函数
        return;
    }

    QSqlQuery query;
    query.prepare("SELECT * FROM pulmonary_artery_pressure WHERE diastolicValue LIKE :keyword OR systolicValue LIKE :keyword OR avgValue LIKE :keyword OR heartbeatValue LIKE :keyword");
    query.bindValue(":keyword", "%" + keyword + "%");
    query.exec();

    qDebug()<<"serchData query has execed";

    // 清空表格
    historyTable->setRowCount(0);

    int row = 0;
    bool found = false;  // 用来检查是否有匹配的数据

    while (query.next()) {
        historyTable->insertRow(row);
        historyTable->setItem(row, 0, new QTableWidgetItem(QString::number(row + 1)));  // 行号
        historyTable->setItem(row, 1, new QTableWidgetItem(query.value("diastolicValue").toString()));
        historyTable->setItem(row, 2, new QTableWidgetItem(query.value("systolicValue").toString()));
        historyTable->setItem(row, 3, new QTableWidgetItem(query.value("avgValue").toString()));
        historyTable->setItem(row, 4, new QTableWidgetItem(query.value("heartbeatValue").toString()));
        historyTable->setItem(row, 5, new QTableWidgetItem(query.value("upload_time").toString()));

        row++;
        found = true;  // 如果查询到数据，更新标记
    }

    // 如果没有找到数据，显示提示框
    if (!found) {
        CustomMessageBox msgBox(this, tr("搜索结果"), tr("没有找到匹配的记录。"), {tr("确定")}, 200);
        msgBox.exec();
    }
    query.finish();
}


void HistoryCheck::deleteData() {
    // 获取选中的行号
    int row = historyTable->currentRow();
    if (row != -1) {
        // 弹出确认窗口，询问是否删除
        CustomMessageBox confirmBox(this, tr("确认删除"), tr("确认删除选中的记录？"), {tr("是"), tr("否")}, 200);
        confirmBox.exec();

        // 如果用户点击了 "Yes"
        QString response = confirmBox.getUserResponse();
        if (response == tr("是")) {
            // 获取行号对应的ID（根据你的表结构，假设是第1列）
            int number = historyTable->item(row, 0)->text().toInt();

            // 在数据库中删除对应的数据
            QSqlQuery query;
            query.prepare("DELETE FROM pulmonary_artery_pressure WHERE number = ?");
            query.addBindValue(number);
            if (query.exec()) {
                qDebug() << "Row deleted successfully!";
                loadHistoryData();  // 删除后重新加载数据
            } else {
                qDebug() << "Failed to delete row: " << query.lastError().text();
            }
        } else {
            qDebug() << "Row deletion cancelled.";
        }
    } else {
        // 如果没有选中行，提示用户
        CustomMessageBox warningBox(this, tr("没有选择"), tr("请选定一行进行删除"), {tr("确定")}, 200);
        warningBox.exec();
    }
}

bool HistoryCheck::eventFilter(QObject *watched, QEvent *event)
{
    // 确保这是监测 searchLineEdit 的事件
    if (watched == searchLineEdit) {
        if (event->type() == QEvent::FocusIn) {
            // 当输入框获得焦点时，显示键盘
            customKeyboard->setVisible(true);

        } else if (event->type() == QEvent::FocusOut) {
            // 当输入框失去焦点时，判断新焦点是否在键盘内
            QWidget *newFocusWidget = QApplication::focusWidget();
            // 如果 newFocusWidget 不在 customKeyboard 里，就隐藏键盘
            if (!customKeyboard->isAncestorOf(newFocusWidget)) {
                customKeyboard->setVisible(false);
            }
        }
    }

    // 交给父类继续处理其他事件
    return QMainWindow::eventFilter(watched, event);
}

void HistoryCheck::onKeyboardKeyPressed(const QString &key)
{
    // 只有当输入框还保留焦点时才处理
//    if (searchLineEdit->hasFocus()) {
        if (key == "Del") {
            // 退格
            searchLineEdit->backspace();
        }
        else if (key == " ") {
            // 插入空格
            searchLineEdit->insert(" ");
        }
        else if (key == "Shift") {
            // 大小写切换，通常由键盘本身处理
            // 如果想在这里处理也可以加逻辑
        }
        else {
            // 普通字符，直接插入
            searchLineEdit->insert(key);
        }
//    }
}

void HistoryCheck::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        qDebug() << "🔄 [HistoryCheck] 语言切换事件触发！";

        // **更新窗口标题**
        this->setWindowTitle(tr("历史查询界面"));

        // **检查指针是否有效，防止空指针访问**
        if (historylabel)
            historylabel->setText(tr("可查询记录"));

        if (deleteButton)
            deleteButton->setText(tr("删除"));

        if (searchButton)
            searchButton->setText(tr("搜索"));

        if (searchLineEdit)
            searchLineEdit->setText(tr("输入关键字..."));

        // **更新表格标题**
        if (historyTable) {
            historyTable->setHorizontalHeaderLabels(QStringList()
                << "NO."
                << tr("舒张压(mmHg)")
                << tr("收缩压(mmHg)")
                << tr("平均值(mmHg)")
                << tr("心率(次/分钟)")
                << tr("上传时间"));
        }

        qDebug() << "✅ [HistoryCheck] UI 语言切换完成！";
    }

    QWidget::changeEvent(event);
}

