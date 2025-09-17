#ifndef MEDICALLOGGER_H
#define MEDICALLOGGER_H

#include <QObject>
#include <QDateTime>
#include <QFile>
#include <QMutex>

/**
 * @class MedicalLogger
 * @brief 医疗系统日志管理类（单例模式）
 *
 * 该类用于医疗设备/软件中的统一日志记录，支持多线程安全写入，
 * 并满足合规要求（如 21 CFR Part 11 审计追踪）。
 *
 * 功能：
 * - 采用单例模式，保证全局唯一实例。
 * - 提供多级别日志记录接口（信息、警告、错误、审计、数据）。
 * - 自动维护日志文件（初始化、关闭、大小检查与轮换）。
 * - 支持 Qt 消息处理器重定向，统一管理 qDebug/qWarning 等输出。
 *
 * 日志级别：
 * - LOG_INFO  ：普通信息（系统运行状态、提示）
 * - LOG_WARN  ：警告信息（异常但不影响运行）
 * - LOG_ERROR ：错误信息（功能受影响，需要关注）
 * - LOG_AUDIT ：审计日志（关键操作，合规必需，需记录 operatorId）
 * - LOG_DATA  ：数据日志（测量/传感器数据记录）
 *
 * 线程安全：
 * - 内部使用 QMutex 确保多线程并发写入日志时的完整性。
 *
 * 用法示例：
 * @code
 * MedicalLogger::instance()->init();
 * MedicalLogger::instance()->writeLog("MeasurementModule",
 *                                     MedicalLogger::LOG_AUDIT,
 *                                     "User started measurement",
 *                                     "operator123", "sensorA");
 * @endcode
 */

class MedicalLogger : public QObject {
    Q_OBJECT
public:
    // 单例模式获取实例
    static MedicalLogger* instance();

    // 日志级别枚举（医疗系统需明确区分日志重要性）
    enum LogLevel {
        LOG_INFO,    // 普通信息
        LOG_WARN,    // 警告（不影响系统运行）
        LOG_ERROR,   // 错误（功能受影响）
        LOG_AUDIT,   // 审计日志（关键操作，符合21 CFR Part 11）
        LOG_DATA     // 数据日志（测量数据记录）
    };

    // 写入日志的核心接口
    // module: 模块名称
    // level: 日志级别
    // content: 日志内容
    // operatorId: 操作员ID（审计必需）
    // sensorId: 关联传感器ID（数据追溯必需）
    void writeLog(const QString& module,
                 LogLevel level,
                 const QString& content,
                 const QString& operatorId = "unknown",
                 const QString& sensorId = "unknown");

    // 初始化日志（创建日志文件，设置路径等）
    void init();

    // 关闭日志文件（清理资源）
    void shutdown();

    // Qt 消息处理器
    static void qtMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);

private:
    // 单例模式：私有构造函数和析构函数
    MedicalLogger(QObject* parent = nullptr);
    ~MedicalLogger() override;

    // 日志文件操作
    QFile m_logFile;

    // 线程安全锁（多线程写入日志时保证完整性）
    QMutex m_logMutex;

    // 记录日志级别
    LogLevel m_logLevel;  // 新增成员变量

    // 辅助函数：将日志级别转换为字符串
    const QString getLevelStr(LogLevel level);

    // 禁止拷贝和赋值（单例模式保护）
    MedicalLogger(const MedicalLogger&) = delete;
    MedicalLogger& operator=(const MedicalLogger&) = delete;

    // 检查日志文件大小并进行日志轮换
    void checkLogFileSize();
};

#endif // MEDICALLOGGER_H
