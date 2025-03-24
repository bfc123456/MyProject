#include "maintenance.h"
#include "CircularProgressBar.h"

MaintenanceWidget::MaintenanceWidget(QWidget *parent) : QWidget(parent) {
    setWindowTitle(tr("Maintenance"));
    setFixedSize(1024, 600);  //让窗口始终固定大小

    //让维护界面始终置顶，并确保它是一个独立的子窗口
    setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint);

    // **创建标题标签**
    titleLabel = new QLabel(tr("Maintenance Mode"), this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: white;");

    // **创建关闭按钮**
    closeButton = new QPushButton(tr("Close"), this);
    closeButton->setStyleSheet("background-color: #FF4C4C; color: white; border-radius: 6px; font-size: 16px;");
    connect(closeButton, &QPushButton::clicked, this, &MaintenanceWidget::closeWidget);

    CircularProgressBar *progressBar = new CircularProgressBar(this);
    progressBar->setGeometry(50, 50, 150, 150); // 设置大小
    progressBar->setProgress(90); // 设置进度 90%

    // **创建主布局**
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(titleLabel);
    layout->addWidget(progressBar);
    layout->addWidget(closeButton);
    layout->setAlignment(Qt::AlignCenter);

    //防止 `QLayout` 影响窗口大小
//    layout->setSizeConstraint(QLayout::SetFixedSize);
    setLayout(layout);

    // **设置背景样式**
//    setStyleSheet("background-color: #222222; border-radius: 10px;");
}

void MaintenanceWidget::closeWidget() {
    this->hide();  //让窗口隐藏，而不是销毁
}
