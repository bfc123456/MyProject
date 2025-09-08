#include "MedicalLogger.h"
#include <QDir>
#include <QDebug>
#include <QMutexLocker>
#include <QDateTime>
#include <QSettings>

const qint64 MAX_LOG_SIZE = 10 * 1024 * 1024;  // 最大日志文件大小（10MB）

// 单例实例初始化
MedicalLogger* MedicalLogger::instance() {
    static MedicalLogger instance;
    return &instance;
}

// 构造函数：初始化日志文件
MedicalLogger::MedicalLogger(QObject* parent) : QObject(parent) {
}

// 析构函数：关闭日志文件
MedicalLogger::~MedicalLogger() {
    if (m_logFile.isOpen()) {
        m_logFile.close();
    }
}

// 初始化日志文件
void MedicalLogger::init() {
    // 从配置文件获取日志目录路径
    QSettings settings("MyCompany", "MyApp");
    QString logDirPath = settings.value("logDir", "C:/MedicalLogs").toString();

    QDir logDir(logDirPath);
    if (!logDir.exists()) {
        if (!logDir.mkpath(".")) {
            qCritical() << "[MedicalLogger] 无法创建日志目录：" << logDirPath;
            return;
        }
    }

    // 生成日志文件名
    QString logFileName = QString("%1/%2.log")
                         .arg(logDirPath)
                         .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd"));
    m_logFile.setFileName(logFileName);

    // 以追加模式打开日志文件
    if (!m_logFile.open(QIODevice::Append | QIODevice::Text)) {
        qCritical() << "[MedicalLogger] 日志文件打开失败：" << m_logFile.errorString()
                   << "，路径：" << logFileName;
    }

    // 从配置获取日志级别
    QString levelStr = settings.value("logLevel", "INFO").toString();
    if (levelStr == "DEBUG") {
        m_logLevel = LOG_INFO;  // 可根据需求调整
    } else if (levelStr == "ERROR") {
        m_logLevel = LOG_ERROR;
    } else {
        m_logLevel = LOG_INFO;
    }
}

//关闭日志文件，清理资源
void MedicalLogger::shutdown(){
    if(m_logFile.isOpen()){
        m_logFile.close();
        qDebug()<<"[MedicalLogger] 日志文件已关闭";
    }
}

// 日志级别转换为字符串
const QString MedicalLogger::getLevelStr(LogLevel level) {
    switch (level) {
        case LOG_INFO:    return "INFO";
        case LOG_WARN:    return "WARN";
        case LOG_ERROR:   return "ERROR";
        case LOG_AUDIT:   return "AUDIT";  // 审计日志需特别标识
        case LOG_DATA:    return "DATA";
        default:          return "UNKNOWN";
    }
}

// 写入日志（线程安全实现）
void MedicalLogger::writeLog(const QString& module,
                            LogLevel level,
                            const QString& content,
                            const QString& operatorId,
                            const QString& sensorId) {
    // 检查日志文件大小并进行管理
    checkLogFileSize();

    // 加锁保证线程安全（多线程环境下日志完整性）
    QMutexLocker locker(&m_logMutex);

    // 检查文件是否打开
    if (!m_logFile.isOpen()) {
        qWarning() << "[MedicalLogger] 日志文件未打开，无法写入日志";
        return;
    }

    // 构建符合医疗追溯要求的日志格式
    QString logLine = QString("[%1] | %2 | %3 | Operator:%4 | Sensor:%5 | %6\n")
                     .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz"))
                     .arg(getLevelStr(level))
                     .arg(module)
                     .arg(operatorId)
                     .arg(sensorId)
                     .arg(content);

    // 写入日志文件并强制刷新（医疗数据需立即落盘，防止丢失）
    QByteArray data = logLine.toUtf8();
    qint64 bytesWritten = m_logFile.write(data);
    if (bytesWritten != data.size()) {
        qCritical() << "[MedicalLogger] 日志写入不完整，预期：" << data.size()
                   << "，实际：" << bytesWritten;
    }
    m_logFile.flush();
}

// 自定义 Qt 消息处理函数
void MedicalLogger::qtMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    QString level;

    // 设置不同的日志级别
    switch (type) {
        case QtDebugMsg:
            level = "DEBUG";
            break;
        case QtInfoMsg:
            level = "INFO";
            break;
        case QtWarningMsg:
            level = "WARN";
            break;
        case QtCriticalMsg:
            level = "ERROR";
            break;
        case QtFatalMsg:
            level = "FATAL";
            break;
    }

    // 在日志中记录消息
    instance()->writeLog("Qt", LOG_INFO, msg, context.function, context.file);  // 记录更多上下文信息
}

// 检查日志文件大小并进行日志轮换
void MedicalLogger::checkLogFileSize() {
    if (m_logFile.size() > MAX_LOG_SIZE) {
        QString oldFileName = m_logFile.fileName();
        QString newFileName = oldFileName + ".old";

        // 关闭当前日志文件
        m_logFile.close();

        // 重命名当前日志文件并创建新日志文件
        if (QFile::rename(oldFileName, newFileName)) {
            m_logFile.setFileName(oldFileName);
            if (!m_logFile.open(QIODevice::Append | QIODevice::Text)) {
                qCritical() << "[MedicalLogger] 新日志文件打开失败：" << m_logFile.errorString();
            }
        } else {
            qCritical() << "[MedicalLogger] 无法重命名日志文件";
        }
    }
}
