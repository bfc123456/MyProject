#include "maintenance.h"

MaintenanceWidget::MaintenanceWidget(QWidget *parent) : QWidget(parent) {
    setWindowTitle(tr("Maintenance"));
    setFixedSize(1024, 600);  //让窗口始终固定大小

    //让维护界面始终置顶，并确保它是一个独立的子窗口
    setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint);

    // **创建标题标签**
    titleLabel = new QLabel(tr("Maintenance Mode"), this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: white;");

    // **创建主布局**
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(titleLabel);  // 添加标题标签
    layout->setAlignment(Qt::AlignCenter);  // 让标题居中

    // 设置布局
    setLayout(layout);

    // **设置背景样式**
    setStyleSheet("background-color: qlineargradient("
                  "x1: 0, y1: 0, x2: 0, y2: 1, "
                  "stop: 0 rgba(30, 50, 80, 0.9),   /* 顶部：偏亮蓝灰，透明度 0.9 */"
                  "stop: 1 rgba(10, 25, 50, 0.75)   /* 底部：深蓝，透明度 0.75 */"
                  ");"
                  "color: white;"
                  "font-size: 14px;"
                  "border-radius: 10px;");
}
MaintenanceWidget::~MaintenanceWidget()
{

}

