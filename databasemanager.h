#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QSqlDatabase>
#include <QString>

/**
 * @class DatabaseManager
 * @brief 数据库管理器（单例模式）
 *
 * 本类用于管理应用程序的数据库连接和操作，采用单例模式确保全局只有一个数据库实例。
 * 提供了打开/关闭数据库、错误信息获取、schema 版本查询等基础功能，
 * 并封装了与传感器相关的数据操作（如插入和查询传感器信息）。
 *
 * 功能特点：
 * - 单例模式，避免多个数据库连接导致资源冲突
 * - 封装 Qt 的 QSqlDatabase 对象，简化数据库操作
 * - 提供统一的错误获取接口，便于日志记录和问题排查
 * - 支持 schema 版本号查询，满足医疗软件的追溯性要求
 * - 内置传感器信息表的插入与查询接口
 *
 * 使用场景：
 * - 系统初始化时建立数据库连接
 * - 在植入注册界面中插入/查询传感器绑定信息
 * - 在测量和回顾界面中读取设备/传感器信息
 *
 * @note 数据库路径需在首次调用 instance() 时传入；
 *       此类仅负责数据库连接与基础操作，不涉及业务逻辑。
 */


class DatabaseManager {
public:
    // 获取单例实例（修正：使用引用避免内存泄漏）
    static DatabaseManager& instance(const QString& dbPath = "");

    // 禁止拷贝构造和赋值操作（单例安全）
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    ~DatabaseManager();

    // 打开数据库连接
    bool openDatabase();

    // 关闭数据库连接
    bool closeDatabase();

    // 获取最后一次错误信息（新增：解决编译错误）
    QString lastError() const;

    // 检查数据库是否已打开
    bool isOpen() const;

    // 获取数据库路径
    QString databasePath() const;

    // 获取数据库 schema 版本（新增：配置追溯用）
    int schemaVersion() const;

    // 传感器信息操作
    bool insertSensorInfo(const QString &sensorId, const QString &calibrationCode,
                          const QString &plantDoctor, const QString &treatmentDoctor,
                          const QString &plantingDate, const QString &location);

    bool querySensorInfo(const QString &sensorId);
    QString getLocationBySensorId(const QString &sensorId);

private:
    // 私有构造函数（单例模式）
    explicit DatabaseManager(const QString &dbPath);

    QSqlDatabase db;          // 数据库连接对象
    QString dbPath;           // 数据库文件路径
    int version;              // 数据库schema版本
};

#endif // DATABASEMANAGER_H
