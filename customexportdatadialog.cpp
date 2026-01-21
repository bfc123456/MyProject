/******************************************************************************
 * 文件名：customexportdatadialog.cpp
 * 功能描述：实现数据导出对话框及后台工作线程，支持原始波形(RAW)与频域数据(FFT)
 * 的二进制异步导出。提供深色现代 UI 交互，支持多屏幕分辨率自适应。
 * 版本：v1.0.0
 * 作者：Gemini (RAG & QT Specialist)
 * 时间：2026/01/15
 * 版本控制：
 * v1.0.0 - 初始版本，实现多线程导出逻辑
 * v1.0.1 - 优化 UI 样式，增加琥珀色暖色调按键，完善学术级代码注释
 * 备注：该类继承自 CloseOnlyWindow，实现了典型的生产者-消费者多线程导出模型。
 ******************************************************************************/

#include "customexportdatadialog.h"
#include "custommessagebox.h"

// Qt Headers
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QCoreApplication>
#include <QScreen>
#include <QGuiApplication>
#include <QDateTime>
#include <QMessageBox>
#include <QDir>
#include <QDebug>
#include <QThread>

// ==========================================
// CustomExportWorker 类实现 (后台文件 IO 处理)
// ==========================================

/***********************************************************************************************
 * FUNC    : CustomExportWorker
 * IN      : const QString &dir - 导出目录, CustomExportType type - 导出模式,
 * const QQueue<QByteArray> &raw - 原始数据, const QQueue<QByteArray> &fft - 频域数据
 * OUT     : None
 * RETURN  : None
 * AUTHOR  : 2026-01-15 Create
 * NOTE    : 构造函数，初始化导出任务所需的上下文数据。
 ************************************************************************************************/
CustomExportWorker::CustomExportWorker(const QString &path, CustomExportType type,
                                       const QVector<quint16> adc, const QVector<quint32> fft)
    : m_savePath(path), m_type(type), m_adcData(adc), m_fftData(fft)
{

}

/***********************************************************************************************
 * FUNC    : process
 * IN      : None
 * OUT     : None
 * RETURN  : void
 * AUTHOR  : 2026-01-15 Create
 * NOTE    : 工作线程主函数。执行二进制文件写入。通过信号 progressChanged 汇报百分比，
 * 通过 finished 信号返回任务最终状态。
 ************************************************************************************************/
void CustomExportWorker::process() {
    if (m_adcData.isEmpty() && m_fftData.isEmpty()) {
        emit finished(false, tr("缓冲区无数据，无法导出"));
        return;
    }

    QDir dir(m_savePath);
    if (!dir.exists()) dir.mkpath(".");
    QString timeStr = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");

    // 2. TXT 写入 Lambda (通用模板)
    auto writeTxtFile = [&](const auto& vectorData, const QString& prefix, const QString& header) -> bool {
        if (vectorData.isEmpty()) return true;

        // 【修改点1】：后缀改为 .txt
        QString fileName = dir.filePath(QString("%1_%2.txt").arg(prefix, timeStr));
        QFile file(fileName);

        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;

        QTextStream out(&file);

        // 【修改点2】：写入头部说明 (可选，不想加头部可以注释掉)
        // out << "# " << header << "\n";

        // 建议保留一行简单的列名，用制表符分隔
        out << "Index\tValue\n";

        for (int i = 0; i < vectorData.size(); ++i) {
            // 【修改点3】：数据之间用制表符 '\t' 分隔，比逗号更适合 Origin/Matlab 读取
            // 格式：索引 [TAB] 数值 [换行]
            out << i << "\t" << vectorData[i] << "\n";

            // 降低主线程进度汇报频率，每 5000 个点汇报一次，提升写入速度
            if (i % 5000 == 0) {
                emit progressChanged(i * 100 / vectorData.size());
            }
        }
        file.close();
        return true;
    };

    // 3. 执行导出
    bool ok = true;
    if (m_type == CustomExportType::RawOnly || m_type == CustomExportType::Both) {
        // 导出 ADC (2字节整数)
        ok &= writeTxtFile(m_adcData, "ADC", "Raw_ADC_Data");
    }
    if (m_type == CustomExportType::FftOnly || m_type == CustomExportType::Both) {
        // 导出 FFT (4字节浮点数)
        ok &= writeTxtFile(m_fftData, "FFT", "FFT_Spectrum_Data");
    }

    emit progressChanged(100);
    emit finished(ok, ok ? tr("导出成功！") : tr("写入失败"));
}

// ==========================================
// CustomExportDataDialog 类实现 (UI 交互控制)
// ==========================================

/***********************************************************************************************
 * FUNC    : CustomExportDataDialog
 * IN      : QWidget *parent - 父窗口指针
 * OUT     : None
 * RETURN  : None
 * AUTHOR  : 2026-01-15 Create
 * NOTE    : 构造函数。执行分辨率适配计算、窗口属性配置及 UI 初始化。
 ************************************************************************************************/
CustomExportDataDialog::CustomExportDataDialog(QWidget *parent)
    : CloseOnlyWindow(parent), m_pWorkerThread(nullptr)
{
    // 获取屏幕分辨率并计算缩放比例
    QScreen *pScreen = QGuiApplication::primaryScreen();
    QRect rectScreenGeometry = pScreen->geometry();
    m_pScaleX = (float)rectScreenGeometry.width() / 1024;
    m_pScaleY = (float)rectScreenGeometry.height() / 600;

    // 窗口基础属性：无边框、透明背景、固定大小
    setFixedSize(int(460 * m_pScaleX), int(350 * m_pScaleY));
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    setupUi();
    applyModernStyle();
}

/***********************************************************************************************
 * FUNC    : ~CustomExportDataDialog
 * IN      : None
 * OUT     : None
 * RETURN  : None
 * AUTHOR  : 2026-01-15 Create
 * NOTE    : 析构函数。安全退出工作线程并释放资源。
 ************************************************************************************************/
CustomExportDataDialog::~CustomExportDataDialog() {
    qDebug() << "[Dialog] 开始进入析构函数, 地址:" << this;
    if (m_pWorkerThread) {
        qDebug() << "[Dialog-Destruct] 检查 Worker 线程状态:" << m_pWorkerThread->isRunning();
        m_pWorkerThread->quit();
        if(!m_pWorkerThread->wait(1000)) {
            qWarning() << "[Dialog-Destruct] Worker 线程强行终止(terminate)";
            m_pWorkerThread->terminate();
        }
        qDebug() << "[Dialog-Destruct] Worker 线程资源已释放";
    }
}

/***********************************************************************************************
 * FUNC    : setupUi
 * IN      : None
 * OUT     : None
 * RETURN  : void
 * AUTHOR  : 2026-01-15 Create
 * NOTE    : 初始化卡片布局、控件配置及信号槽连接。
 ************************************************************************************************/
void CustomExportDataDialog::setupUi() {
    // 初始化卡片主体
    m_pCardFrame = new QFrame(this);
    m_pCardFrame->setObjectName("cardFrame");
    m_pCardFrame->setFixedSize(this->size());

    QVBoxLayout *pMainLayout = new QVBoxLayout(m_pCardFrame);
    pMainLayout->setContentsMargins(22 * m_pScaleX, 18 * m_pScaleY, 22 * m_pScaleX, 18 * m_pScaleY);
    pMainLayout->setSpacing(15 * m_pScaleY);

    // 1. 标题栏
    QHBoxLayout *pTopBar = new QHBoxLayout();
    m_pTitleLbl = new QLabel(tr("导出测量数据"), m_pCardFrame);
    m_pTitleLbl->setObjectName("titleLabel");

    // 复用 CloseOnlyWindow 可能有的关闭逻辑，或自定义
    QPushButton *pCloseBtn = new QPushButton(m_pCardFrame);
    pCloseBtn->setObjectName("closeBtn");
    pCloseBtn->setFixedSize(30 * m_pScaleX, 30 * m_pScaleX);
    pCloseBtn->setIcon(QIcon(":/image/icons-close.png")); // 确保资源路径正确
    pCloseBtn->setIconSize(QSize(18 * m_pScaleX, 18 * m_pScaleX));

    pTopBar->addWidget(m_pTitleLbl);
    pTopBar->addStretch();
    pTopBar->addWidget(pCloseBtn);
    pMainLayout->addLayout(pTopBar);

    // 2. 导出内容选择框 (采用半透明背景块)
    QFrame *pContentFrame = new QFrame(m_pCardFrame);
    pContentFrame->setObjectName("contentFrame");
    QVBoxLayout *pContentLayout = new QVBoxLayout(pContentFrame);
    pContentLayout->setSpacing(10 * m_pScaleY);

    m_pRbRaw = new QRadioButton(tr("仅导出原始波形 (Raw Data)"), pContentFrame);
    m_pRbFft = new QRadioButton(tr("仅导出频域数据 (FFT Data)"), pContentFrame);
    m_pRbBoth = new QRadioButton(tr("导出全部数据 (Raw + FFT)"), pContentFrame);
    m_pRbBoth->setChecked(true);

    m_pTypeGroup = new QButtonGroup(this);
    m_pTypeGroup->addButton(m_pRbRaw, 0);
    m_pTypeGroup->addButton(m_pRbFft, 1);
    m_pTypeGroup->addButton(m_pRbBoth, 2);

    pContentLayout->addWidget(m_pRbRaw);
    pContentLayout->addWidget(m_pRbFft);
    pContentLayout->addWidget(m_pRbBoth);
    pMainLayout->addWidget(pContentFrame);

    // 3. 路径选择
    QHBoxLayout *pPathLayout = new QHBoxLayout();
    m_pPathEdit = new QLineEdit(QCoreApplication::applicationDirPath() + "/data", m_pCardFrame);
    m_pPathEdit->setReadOnly(true);
    m_pPathEdit->setFixedHeight(36 * m_pScaleY);

    m_pBrowseBtn = new QPushButton(tr("浏览..."), m_pCardFrame);
    m_pBrowseBtn->setObjectName("browseBtn");
    m_pBrowseBtn->setFixedSize(80 * m_pScaleX, 36 * m_pScaleY);

    pPathLayout->addWidget(m_pPathEdit);
    pPathLayout->addWidget(m_pBrowseBtn);
    pMainLayout->addLayout(pPathLayout);

    // 4. 状态描述与进度条
    m_pStatusLabel = new QLabel(tr("准备就绪"), m_pCardFrame);
    m_pStatusLabel->setStyleSheet("color: rgba(255,255,255,160); font-size: 13px;");

    m_pProgressBar = new QProgressBar(m_pCardFrame);
    m_pProgressBar->setFixedHeight(4 * m_pScaleY);
    m_pProgressBar->setTextVisible(false);
    m_pProgressBar->hide();

    pMainLayout->addStretch();
    pMainLayout->addWidget(m_pStatusLabel);
    pMainLayout->addWidget(m_pProgressBar);

    // 5. 底部按钮
    QHBoxLayout *pBtnLayout = new QHBoxLayout();
    m_pCancelBtn = new QPushButton(tr("取消"), m_pCardFrame);
    m_pCancelBtn->setObjectName("cancelBtn");
    m_pStartBtn = new QPushButton(tr("开始导出"), m_pCardFrame);
    m_pStartBtn->setObjectName("startBtn");
    m_pCancelBtn->setFixedSize(120 * m_pScaleX, 45 * m_pScaleY);
    m_pStartBtn->setFixedSize(120 * m_pScaleX, 45 * m_pScaleY);

    pBtnLayout->addWidget(m_pCancelBtn);
    pBtnLayout->addStretch();
    pBtnLayout->addWidget(m_pStartBtn);
    pMainLayout->addLayout(pBtnLayout);

    // 连接基本 UI 信号
    connect(m_pBrowseBtn, &QPushButton::clicked, this, &CustomExportDataDialog::onBrowseClicked);
    connect(m_pStartBtn, &QPushButton::clicked, this, &CustomExportDataDialog::onStartClicked);
    connect(m_pCancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(pCloseBtn, &QPushButton::clicked, this, &CustomExportDataDialog::close);
}

/***********************************************************************************************
 * FUNC    : applyModernStyle
 * IN      : None
 * OUT     : None
 * RETURN  : void
 * AUTHOR  : 2026-01-15 Create
 * NOTE    : 加载 QSS 样式表。定义深色主题、琥珀色浏览按钮及无轮廓关闭按钮。
 ************************************************************************************************/
void CustomExportDataDialog::applyModernStyle() {
    // 基础按钮样式定义
    QString strButtonStyle = R"(
        QPushButton {
            background-color: #333333; /* 默认深灰 */
            border: 1px solid rgba(255,255,255,40);
            border-radius: 6px;
            color: #DCDCDC;
            font-weight: bold;
            font-size: 14px;
        }
        QPushButton:hover { background-color: #444444; }

        /* 1. 浏览按钮 - 暖色调 (琥珀橙) */
         QPushButton#browseBtn {
             background-color: #FF9800; /* 暖橙色 */
             color: #1E1E1E;            /* 深色文字对比更清晰 */
             border: none;
             border-radius: 6px;
             font-weight: bold;
         }
         QPushButton#browseBtn:hover {
             background-color: #FFA726; /* 悬停稍亮 */
         }
         QPushButton#browseBtn:pressed {
             background-color: #F57C00; /* 按下稍暗 */
         }

        /* 2. 开始导出 - 科技蓝 (核心动作) */
        QPushButton#startBtn {
            background-color: #2196F3;
            color: white;
            border: none;
        }
        QPushButton#startBtn:hover { background-color: #42A5F5; }
        QPushButton#startBtn:disabled { background-color: #444; color: #888; }

        /* 3. 取消按钮 - 幽灵样式 */
        QPushButton#cancelBtn {
            background-color: transparent;
            border: 1px solid rgba(255,255,255,60);
        }
        QPushButton#cancelBtn:hover { background-color: rgba(255,255,255,20); }

        /* 4. 关闭按钮 - 彻底无轮廓设计 */
        QPushButton#closeBtn {
             background: transparent;
             border: none;
             outline: none;
             /* 默认半透明，让它不那么刺眼，融入背景 */
             qproperty-icon: url(":/image/icons-close.png"); /* 确保路径正确 */
             padding: 4px;
         }

        QPushButton#closeBtn:hover {
             /* 悬停时通过背景色的极低透明度或者干脆不设背景，只变图标亮度 */
             background: rgba(255, 255, 255, 15); /* 极淡的反馈，几乎看不出轮廓 */
             border-radius: 15px;
         }

        QPushButton#closeBtn:pressed {
             background: rgba(255, 255, 255, 5);
         }
    )";

    QString strStyle = R"(
        #cardFrame {
            background-color: #262A33;
            border: 1px solid rgba(255,255,255,90);
            border-radius: 16px;
        }
        #contentFrame {
            background-color: rgba(0,0,0,40);
            border-radius: 10px;
        }
        QLabel { background: transparent; color: white; }

        QRadioButton { color: rgba(255,255,255,220); font-size: 14px; spacing: 10px; }
        QRadioButton::indicator {
            width: 18px; height: 18px;
            border-radius: 10px;
            border: 2px solid rgba(255,255,255,60);
        }

        /* 标题标签提亮加粗 */
        QLabel#titleLabel {
            font-size: 20px;
            font-weight: 900;      /* 极粗体 */
            color: #FFFFFF;        /* 提亮至纯白 */
            background: transparent;
            /* 在深色背景下，纯白文字配合 900 字重会非常有张力 */
       }
        QRadioButton::indicator:checked { background: #2196F3; border: 4px solid #262A33; }

        QLineEdit {
            border: 1px solid rgba(255,255,255,60);
            border-radius: 8px;
            padding: 0 12px;
            background-color: rgba(0,0,0,60);
            color: #FFFFFF;
            font-size: 14px;
        }

        QProgressBar { background-color: rgba(255,255,255,30); border: none; border-radius: 2px; }
        QProgressBar::chunk { background-color: #4CAF50; }
    )" + strButtonStyle;

    this->setStyleSheet(strStyle);
}

/***********************************************************************************************
 * FUNC    : onBrowseClicked
 * IN      : None
 * OUT     : None
 * RETURN  : void
 * AUTHOR  : 2026-01-15 Create
 * NOTE    : 浏览按钮槽函数。调用标准目录选择框。
 ************************************************************************************************/
void CustomExportDataDialog::onBrowseClicked() {
    QString dir = QFileDialog::getExistingDirectory(this, tr("选择保存目录"), m_pPathEdit->text());
    if (!dir.isEmpty()) {
        m_pPathEdit->setText(dir);
    }
}

/***********************************************************************************************
 * FUNC    : onStartClicked
 * IN      : None
 * OUT     : None
 * RETURN  : void
 * AUTHOR  : 2026-01-15 Create
 * NOTE    : 开始导出按钮槽函数。禁用 UI 控件并发出 requestData 信号。
 ************************************************************************************************/
void CustomExportDataDialog::onStartClicked() {
    // 锁定界面防止重复操作
    m_pStartBtn->setEnabled(false);
    m_pCancelBtn->setEnabled(false);
    m_pBrowseBtn->setEnabled(false);
    m_pRbRaw->setEnabled(false);
    m_pRbFft->setEnabled(false);
    m_pRbBoth->setEnabled(false);

    m_pStatusLabel->setText(tr("正在获取系统数据..."));
    m_pProgressBar->show();
    m_pProgressBar->setValue(0);

    // 请求主线程塞入数据
    emit requestData();
}

/***********************************************************************************************
 * FUNC    : startExportTask
 * IN      : const QQueue<QByteArray> &raw, const QQueue<QByteArray> &fft
 * OUT     : None
 * RETURN  : void
 * AUTHOR  : 2026-01-15 Create
 * NOTE    : 外部接口，接收主线程数据并启动后台 Worker 线程开始 IO。
 ************************************************************************************************/
void CustomExportDataDialog::startExportTask(const QVector<quint16>& adcData, const QVector<quint32>& fftData) {
    m_pStatusLabel->setText(tr("正在写入文件，请勿关闭..."));

    m_pWorkerThread = new QThread;
    CustomExportWorker *worker = new CustomExportWorker(getSavePath(), getSelectedType(), adcData, fftData);
    worker->moveToThread(m_pWorkerThread);

    connect(m_pWorkerThread, &QThread::started, worker, &CustomExportWorker::process);
    connect(worker, &CustomExportWorker::progressChanged, this, &CustomExportDataDialog::onProgressUpdate);
    connect(worker, &CustomExportWorker::finished, this, &CustomExportDataDialog::onExportFinished);

    // 线程资源回收逻辑
    connect(worker, &CustomExportWorker::finished, m_pWorkerThread, &QThread::quit);
    connect(worker, &CustomExportWorker::finished, worker, &CustomExportWorker::deleteLater);
    connect(m_pWorkerThread, &QObject::destroyed, this, [=]() {
            this->m_pWorkerThread = nullptr;
            qDebug() << "[Thread-Safety] 线程对象内存已物理释放，指针已置空";
        });

    m_pWorkerThread->start();
}

/***********************************************************************************************
 * FUNC    : onProgressUpdate
 * IN      : int value - 当前进度百分比
 * OUT     : None
 * RETURN  : void
 * AUTHOR  : 2026-01-15 Create
 * NOTE    : 更新界面进度条。
 ************************************************************************************************/
void CustomExportDataDialog::onProgressUpdate(int value) {
    m_pProgressBar->setValue(value);
}

/***********************************************************************************************
 * FUNC    : onExportFinished
 * IN      : bool success - 是否成功, const QString &msg - 提示信息
 * OUT     : None
 * RETURN  : void
 * AUTHOR  : 2026-01-15 Create
 * NOTE    : 任务结束槽函数。根据成功与否提示用户并恢复 UI 状态或关闭对话框。
 ************************************************************************************************/
void CustomExportDataDialog::onExportFinished(bool success, const QString &msg) {

    // 进度条拉满并更新状态文字
    m_pProgressBar->setValue(100);
    this->hide();
    m_pStatusLabel->setText(success ? tr("导出完成") : tr("导出失败"));

    // 1. 在栈上创建提示框：用户不点掉，代码不会往下走，绝对同步
    CustomMessageBox msgBox(this, success ? tr("导出成功") : tr("导出失败"), msg, {tr("确定")}, 400);
    msgBox.exec();

    // 2. 任务分工：对话框只负责改变状态，不负责 delete 自己
    if (success) {
        this->accept(); // 触发信号，通知主界面开始“打扫战场”
    } else {
        // 导出失败，恢复 UI 允许用户修改路径后重试
        this->show();
        m_pStartBtn->setEnabled(true);
        m_pCancelBtn->setEnabled(true);
        m_pBrowseBtn->setEnabled(true);
        m_pRbRaw->setEnabled(true);
        m_pRbFft->setEnabled(true);
        m_pRbBoth->setEnabled(true);
        m_pProgressBar->hide();
    }
}

/***********************************************************************************************
 * FUNC    : getSelectedType
 * IN      : None
 * OUT     : None
 * RETURN  : CustomExportType - 选中类型枚举
 * AUTHOR  : 2026-01-15 Create
 * NOTE    : 获取用户当前选定的导出内容模式。
 ************************************************************************************************/
CustomExportType CustomExportDataDialog::getSelectedType() const {
    int id = m_pTypeGroup->checkedId();
    return static_cast<CustomExportType>(id);
}

/***********************************************************************************************
 * FUNC    : getSavePath
 * IN      : None
 * OUT     : None
 * RETURN  : QString - 路径字符串
 * AUTHOR  : 2026-01-15 Create
 * NOTE    : 获取 QLineEdit 内部的导出路径。
 ************************************************************************************************/
QString CustomExportDataDialog::getSavePath() const {
    return m_pPathEdit->text();
}
