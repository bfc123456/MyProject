#include "readoutrecorddialog.h"
#include <QGuiApplication>
#include <QScreen>

ReadoutRecordDialog::ReadoutRecordDialog(QWidget *parent) : QDialog(parent) {

    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int screenWidth = screenGeometry.width();
    int screenHeight = screenGeometry.height();

    // 计算缩放比例
    scaleX = (float)screenWidth / 1024;
    scaleY = (float)screenHeight / 600;

    this->setWindowTitle("读数记录");
    this->setFixedSize(800*scaleX, 300*scaleY);
    this->setObjectName("mainwidget");
    this->setStyleSheet(R"(
    QWidget#mainwidget{
    background-color: qlineargradient(
        x1: 0, y1: 0, x2: 0, y2: 1,
        stop: 0 rgba(40, 55, 80, 230),
        stop: 1 rgba(25, 35, 55, 230)
    );
    border: 1px solid rgba(255, 255, 255, 0.06);
    border-radius: 10px;
    }
    )");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 顶部标题与按钮
    QLabel *titleLabel = new QLabel("读数记录");
    titleLabel->setStyleSheet("font-weight: bold; font-size: 16px; background-color: transparent; color: white;");
    btnRefresh = new QPushButton("重新校准");
    btnRefresh->setIcon(QIcon(":/image/icons8-calibration.png"));
    btnRefresh->setFixedSize(120*scaleX, 40*scaleY);
    btnRefresh->setStyleSheet(R"(
        QPushButton {
            background-color: rgba(255, 255, 255, 0.3);           /* 半透明浅灰 */
            border: 1px solid rgba(255, 255, 255, 0.4);            /* 高亮边框 */
            border-radius: 32px;                                  /* 圆角按钮 */
            color: white;
            font-weight: bold;
            font-size: 14px;
            padding: 6px 16px;
        }

        QPushButton:pressed {
            background-color: rgba(200, 200, 200, 0.4);            /* 按下偏深 */
            padding-left: 8px;
            padding-top: 7px;
        }
    )");

    btnDelete = new QPushButton("删除");
    btnDelete->setFixedSize(120*scaleX, 40*scaleY);
    btnDelete->setIcon(QIcon(":/image/delete.png"));
    btnDelete->setStyleSheet(R"(
        QPushButton {
            background-color: rgba(255, 100, 100, 0.25);           /* 半透明浅红 */
            border: 1px solid rgba(255, 120, 120, 0.4);             /* 柔和红色边框 */
            border-radius: 32px;                                  /* 圆角按钮 */
            color: white;
            font-weight: bold;
            font-size: 14px;
            padding: 6px 16px;
        }

        QPushButton:pressed {
            background-color: rgba(200, 200, 200, 0.4);            /* 按下偏深 */
            padding-left: 8px;
            padding-top: 7px;
        }
    )");
    connect(btnDelete, &QPushButton::clicked,this,&ReadoutRecordDialog::onDeleteButtonClicked);
    connect(btnRefresh, &QPushButton::clicked,this,[this](){
        emit onRefreshButtonClicked(); 
        this->close();
    });

    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->addWidget(titleLabel);
    topLayout->addStretch();
    topLayout->addWidget(btnRefresh);
    topLayout->addSpacing(10*scaleX);
    topLayout->addWidget(btnDelete);

    // 表格
    table = new QTableWidget(0, 5, this);
    QStringList headers = {"序号", "舒张压", "收缩压", "平均值", "心率"};
    table->setHorizontalHeaderLabels(headers);

    auto *h = table->horizontalHeader();

    // 0、1、2 列：Interactive (或者 Fixed)，并手动设置初始宽度
    h->setSectionResizeMode(0, QHeaderView::Stretch);
    h->setSectionResizeMode(1, QHeaderView::Stretch);
    h->setSectionResizeMode(2, QHeaderView::Stretch);
    h->setSectionResizeMode(3, QHeaderView::Stretch);
    h->setSectionResizeMode(4, QHeaderView::Stretch);

    table->verticalHeader()->setVisible(false); //隐藏左侧行号

    table->setEditTriggers(QAbstractItemView::NoEditTriggers);  //关闭所有编辑触发器
    table->setStyleSheet(R"(
        QTableWidget {
            background-color: #1e1e1e;           /* 深灰黑背景 */
            color: #ffffff;                      /* 白色文字 */
            gridline-color: #2e2e2e;             /* 表格线为深灰 */
            selection-background-color: #3a3a3a; /* 选中行背景 */
            border-radius: 6px;
        }

        QHeaderView::section {
            background-color: #2a2a2a;
            color: #ffffff;
            padding: 4px;
            border: none;
            font-weight: bold;
        }

        QTableWidget QTableCornerButton::section {
            background-color: #2a2a2a;
            border: none;
        }
    )");


    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(table);
}

ReadoutRecordDialog::~ ReadoutRecordDialog(){
    
}

// 获取量测信息更新至列表
void ReadoutRecordDialog::populateData(const QList<MeasurementData> &list) {
    table->clearContents();
    table->setRowCount(list.size());
    for (int i = 0; i < list.size(); ++i) {
        const auto &d = list[i];

        auto makeCenteredItem = [&](const QString &text) {
            auto *item = new QTableWidgetItem(text);
            item->setTextAlignment(Qt::AlignCenter);
            return item;
        };

        table->setItem(i, 0, makeCenteredItem(QString::number(d.order)));
        table->setItem(i, 1, makeCenteredItem(d.sensorSystolic));
        table->setItem(i, 2, makeCenteredItem(d.sensorDiastolic));
        table->setItem(i, 3, makeCenteredItem(d.sensorAvg));
        table->setItem(i, 4, makeCenteredItem(d.heartRate));
    }
}

void ReadoutRecordDialog::onDeleteButtonClicked()
{
    int row = table->currentRow();
    if (row < 0) {
        // 没有选中任何行
        return;
    }
    // 通知外面要删除哪一行
    emit rowDeleted(row);
}
