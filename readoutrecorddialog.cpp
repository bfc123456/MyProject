#include "readoutrecorddialog.h"

ReadoutRecordDialog::ReadoutRecordDialog(QWidget *parent) : QDialog(parent) {
    this->setWindowTitle("读数记录");
    this->setFixedSize(800, 300);
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
    titleLabel->setStyleSheet("font-weight: bold; font-"
                              "size: 16px; background-color: transparent; color: white;");
    btnRefresh = new QPushButton("重新校准");
    btnRefresh->setIcon(QIcon(":/image/icons8-calibration.png"));
    btnRefresh->setFixedSize(120, 40);
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
    btnDelete->setFixedSize(120, 40);
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


    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->addWidget(titleLabel);
    topLayout->addStretch();
    topLayout->addWidget(btnRefresh);
    topLayout->addSpacing(10);
    topLayout->addWidget(btnDelete);

    // 表格
    table = new QTableWidget(0, 5, this);
    QStringList headers = {"序号", "传感器", "参考值", "心率", "位置"};
    table->setHorizontalHeaderLabels(headers);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setVisible(false);
    table->setSelectionMode(QAbstractItemView::NoSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
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

    // 插入测试数据
    table->insertRow(0);
    table->setItem(0, 0, new QTableWidgetItem("1"));
    table->setItem(0, 1, new QTableWidgetItem("120  80  (93)"));
    table->setItem(0, 2, new QTableWidgetItem("120  80  (93)"));
    table->setItem(0, 3, new QTableWidgetItem("75"));
    table->setItem(0, 4, new QTableWidgetItem("0°"));
}

ReadoutRecordDialog::~ ReadoutRecordDialog(){
    
}

void ReadoutRecordDialog::populateData() {
    table->insertRow(0);
    table->setItem(0, 0, new QTableWidgetItem("1"));
    table->setItem(0, 1, new QTableWidgetItem("120  80  (93)"));
    table->setItem(0, 2, new QTableWidgetItem("120  80  (93)"));
    table->setItem(0, 3, new QTableWidgetItem("75"));
    table->setItem(0, 4, new QTableWidgetItem("0°"));
}
