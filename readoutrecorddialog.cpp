#include "readoutrecorddialog.h"
#include <QGuiApplication>
#include <QScreen>
#include <QGraphicsDropShadowEffect>
#include <QFrame>


ReadoutRecordDialog::ReadoutRecordDialog(QWidget *parent)
    : CloseOnlyWindow(parent)
{
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int screenWidth = screenGeometry.width();
    int screenHeight = screenGeometry.height();

    scaleX = (float)screenWidth / 1024;
    scaleY = (float)screenHeight / 600;

    // ====== 透明外层 + 无边框（统一你之前弹窗风格）======
    setFixedSize(820*scaleX, 360*scaleY);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setObjectName("ReadoutRecordDialog");
    setStyleSheet(R"(
        #ReadoutRecordDialog{ background: transparent; }
    )");

    // ====== Root：只负责居中放卡片 ======
    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setAlignment(Qt::AlignCenter);

    // ====== Card：真正的窗口主体 ======
    QFrame *card = new QFrame(this);
    card->setObjectName("card");
    card->setFixedSize(size()); // 与窗口同大小（你如果想留阴影空间可以略缩小）
    card->setStyleSheet(R"(
        #card{
            background-color: #262A33;
            border: 1px solid rgba(255,255,255,90);
            border-radius: 16px;
        }
        #card QLabel{
            color: rgba(255,255,255,220);
            background: transparent;
        }
    )");

    auto *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(26);
    shadow->setOffset(0, 10);
    shadow->setColor(QColor(0,0,0,180));
    card->setGraphicsEffect(shadow);

    root->addWidget(card);

    const int r = 16; // 圆角半径，和 QSS 里一致

    QPainterPath path;
    path.addRoundedRect(card->rect(), r, r);
    setMask(QRegion(path.toFillPolygon().toPolygon()));

    QVBoxLayout *mainLayout = new QVBoxLayout(card);
    mainLayout->setContentsMargins(22*scaleX, 18*scaleY, 22*scaleX, 18*scaleY);
    mainLayout->setSpacing(12*scaleY);

    // ====== 顶栏：标题 + 关闭按钮（统一幽灵风格）======
    titleLabel = new QLabel(tr("读数记录"), card);
    titleLabel->setStyleSheet("font-size:22px; font-weight:800;");

    QPushButton *closeButton = new QPushButton(card);
    closeButton->setObjectName("closeBtn");
    closeButton->setIcon(QIcon(":/image/icons-close.png"));                 // ARM 更稳（不要用 ✕）
    closeButton->setIconSize(QSize(20*scaleX,20*scaleX));
    closeButton->setFixedSize(28*scaleX, 28*scaleX);
    closeButton->setCursor(Qt::PointingHandCursor);
    closeButton->setStyleSheet(R"(
        #closeBtn{
            background: transparent;
            border: none;
            border-radius: 14px;
            color: rgba(255,255,255,180);
            font-weight: 800;
            font-size: 14px;
        }
        #closeBtn:hover{ background: rgba(255,255,255,40); color: white; }
        #closeBtn:pressed{ background: rgba(0,0,0,70); }
    )");

    // ====== 工具栏：按钮放这里，比挤在标题栏舒服 ======
    btnRefresh = new QPushButton(tr("重新校准"), card);
    btnRefresh->setIcon(QIcon(":/image/icons8-calibration.png"));
    btnRefresh->setFixedSize(140*scaleX, 40*scaleY);
    btnRefresh->setCursor(Qt::PointingHandCursor);

    btnDelete = new QPushButton(tr("删除"), card);
    btnDelete->setIcon(QIcon(":/image/delete.png"));
    btnDelete->setFixedSize(140*scaleX, 40*scaleY);
    btnDelete->setCursor(Qt::PointingHandCursor);

    // 统一按钮体系：主按钮蓝，危险按钮红（更现代）
    QString primaryBtn = R"(
        QPushButton{
            background-color: #1E8CFF;
            border: none;
            border-radius: 10px;
            color: white;
            font-weight: 700;
            font-size: 14px;
            padding: 6px 16px;
        }
        QPushButton:hover{ background-color: #3C9DFF; }
        QPushButton:pressed{ background-color: #1673D2; }
    )";
    QString dangerBtn = R"(
        QPushButton{
            background-color: rgba(255, 80, 80, 0.22);
            border: 1px solid rgba(255, 120, 120, 0.45);
            border-radius: 10px;
            color: rgba(255,255,255,230);
            font-weight: 700;
            font-size: 14px;
            padding: 6px 16px;
        }
        QPushButton:hover{ background-color: rgba(255, 80, 80, 0.30); }
        QPushButton:pressed{ background-color: rgba(255, 80, 80, 0.38); }
    )";
    btnRefresh->setStyleSheet(primaryBtn);
    btnDelete->setStyleSheet(dangerBtn);

    QHBoxLayout *topBarLayout = new QHBoxLayout();
    topBarLayout->setContentsMargins(0, 0, 0, 0);
    topBarLayout->setSpacing(12 * scaleX);

    topBarLayout->addStretch();
    topBarLayout->addWidget(titleLabel);
    topBarLayout->addStretch();
    mainLayout->addLayout(topBarLayout);

    // 关闭按钮与操作按钮略微隔开
    topBarLayout->addSpacing(6 * scaleX);
    topBarLayout->addWidget(closeButton);

    // ====== 表格容器：给表格“卡片感”，不会像黑洞 ======
    QFrame *tableWrap = new QFrame(card);
    tableWrap->setObjectName("tableWrap");
    tableWrap->setStyleSheet(R"(
        #tableWrap{
            background-color: rgba(0,0,0,0.22);
            border: 1px solid rgba(255,255,255,40);
            border-radius: 12px;
        }
    )");
    QVBoxLayout *wrapLayout = new QVBoxLayout(tableWrap);
    wrapLayout->setContentsMargins(10*scaleX, 10*scaleY, 10*scaleX, 10*scaleY);

    table = new QTableWidget(0, 5, tableWrap);
    QStringList headers = {tr("序号"), tr("舒张压"), tr("收缩压"), tr("平均值"), tr("心率")};
    table->setHorizontalHeaderLabels(headers);

    auto *h = table->horizontalHeader();
    h->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setVisible(false);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setShowGrid(false);

    table->setStyleSheet(R"(
        QTableWidget{
            background: transparent;
            color: rgba(255,255,255,220);
            border: none;
        }
        QHeaderView::section{
            background-color: rgba(255,255,255,0.06);
            color: rgba(255,255,255,200);
            padding: 8px;
            border: none;
            font-weight: 700;
        }
        QTableWidget::item{
            padding: 8px;
        }
        QTableWidget::item:selected{
            background-color: rgba(30,140,255,0.22);
        }
    )");

    wrapLayout->addWidget(table);
    mainLayout->addWidget(tableWrap);

    QHBoxLayout *bottomLayout = new QHBoxLayout();
    bottomLayout->setContentsMargins(0, 8*scaleY, 0, 0);
    bottomLayout->setSpacing(12*scaleX);

    bottomLayout->addWidget(btnRefresh);
    bottomLayout->addStretch();
    bottomLayout->addWidget(btnDelete);

    mainLayout->addLayout(bottomLayout);

    // ====== 连接（保持你原功能）======
    connect(btnDelete, &QPushButton::clicked, this, &ReadoutRecordDialog::onDeleteButtonClicked);
    connect(btnRefresh, &QPushButton::clicked, this, [this](){
        emit onRefreshButtonClicked();
        this->close();
    });
    connect(closeButton, &QPushButton::clicked, this, &ReadoutRecordDialog::close);
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

void ReadoutRecordDialog::changeEvent(QEvent *event){
    QWidget::changeEvent(event);
    if (event->type() == QEvent::LanguageChange) {
        // 系统自动发送的 LanguageChange
        titleLabel->setText(tr("读数记录"));
        btnRefresh->setText(tr("重新校准"));
        btnDelete->setText(tr("删除"));

    if (table) {
        // 重新生成翻译后的表头列表（与初始化时的表头顺序一致）
        QStringList newHeaders = {
            tr("序号"),
            tr("舒张压"),
            tr("收缩压"),
            tr("平均值"),
            tr("心率")
        };
        // 覆盖旧表头，应用新翻译的表头
        table->setHorizontalHeaderLabels(newHeaders);
    }
    }
}
