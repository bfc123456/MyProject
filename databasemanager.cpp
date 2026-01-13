#include "DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QStandardPaths>
#include <QDir>

DatabaseManager::DatabaseManager(const QString &dbPath)
    : dbPath(dbPath)
{
    db = QSqlDatabase::addDatabase("QSQLITE");  // 使用 SQLite 数据库
    db.setDatabaseName(dbPath);  // 设置数据库文件路径
}

DatabaseManager& DatabaseManager::instance(const QString& dbPath)
{
    static DatabaseManager* inst = nullptr;
    static QString chosenPath;   // 记录首次确定的最终路径

    // 1) 计算这次调用“想要”的最终路径（但不一定会生效）
    QString finalPath;

    if (dbPath.isEmpty()) {
        finalPath = ":/database/MyDatabase.db";
    } else {
        finalPath = dbPath;
    }

    // 2) 如果是资源路径，复制到可写目录
    if (finalPath.startsWith(":/")) {
        const QString writablePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir(writablePath).mkpath(".");
        const QString localDbPath = QDir(writablePath).filePath("MyDatabase.db");

        if (!QFile::exists(localDbPath)) {
            if (!QFile::copy(finalPath, localDbPath)) {
                qWarning() << "[DB] Failed to copy db from resource to:" << localDbPath;
                qWarning() << "[DB] Resource path =" << finalPath;
            } else {
                QFile::setPermissions(localDbPath, QFile::ReadUser | QFile::WriteUser);
            }
        }

        finalPath = localDbPath;
    }

    // 3) 第一次创建实例
    if (!inst) {
        chosenPath = finalPath;
        qDebug() << "[DB] Create singleton with path:" << chosenPath;

        inst = new DatabaseManager(chosenPath);
        if (!inst->openDatabase()) {
            qWarning() << "[DB] openDatabase failed at create, path =" << chosenPath;
        }
        return *inst;
    }

    // 4) 之后再次调用：如果传入不同路径，提示“冲突”，不允许悄悄换
    if (!dbPath.isEmpty() && finalPath != chosenPath) {
        qWarning() << "[DB] DatabaseManager::instance called with DIFFERENT path!";
        qWarning() << "[DB] Ignored new path =" << finalPath;
        qWarning() << "[DB] Using chosenPath =" << chosenPath;
    }

    // 5) 防御：如果意外关闭了，重新打开（可选）
    if (!inst->db.isOpen()) {
        qWarning() << "[DB] db is closed, reopen. path =" << chosenPath;
        inst->openDatabase();
    }

    return *inst;
}

DatabaseManager::~DatabaseManager()
{
    closeDatabase();  // 析构时关闭数据库连接
}

bool DatabaseManager::openDatabase()
{
    if (!db.open()) {
        qDebug() << "Error opening database:" << db.lastError().text();
        return false;
    }

    // 打开数据库后，立即开启外键约束检查
    QSqlQuery pragma(db);
    if (!pragma.exec("PRAGMA foreign_keys = ON;")) {
        qWarning() << "Failed to enable foreign key support:"
                   << pragma.lastError().text();
    }

    qDebug() << "Database opened successfully!";
    return true;
}

bool DatabaseManager::closeDatabase()
{
    if (db.isOpen()) {
        db.close();
        qDebug() << "Database closed.";
        return true;
    }
    return false;
}

bool DatabaseManager::insertSensorInfo(const QString &sensorId, const QString &calibrationCode,
                                       const QString &plantDoctor, const QString &treatmentDoctor,
                                       const QString &plantingDate, const QString &location)
{
    QSqlQuery query;
    query.prepare("INSERT INTO sensor_info (sensor_id, calibration_code, plant_doctor, treatment_doctor, planting_date, location) "
                  "VALUES (:sensorId, :calibrationCode, :plantDoctor, :treatmentDoctor, :plantingDate, :location)");

    query.bindValue(":sensorId", sensorId);
    query.bindValue(":calibrationCode", calibrationCode);
    query.bindValue(":plantDoctor", plantDoctor);
    query.bindValue(":treatmentDoctor", treatmentDoctor);
    query.bindValue(":plantingDate", plantingDate);
    query.bindValue(":location", location);

    if (!query.exec()) {
        qDebug() << "Error inserting data:" << query.lastError().text();
        return false;  // 插入失败
    }

    qDebug() << "Data inserted successfully.";
    return true;  // 插入成功
}

bool DatabaseManager::querySensorInfo(const QString &sensorId)
{
    QSqlQuery query;
    query.prepare("SELECT * FROM sensor_info WHERE sensor_id = :sensorId");
    query.bindValue(":sensorId", sensorId);

    if (!query.exec()) {
        qDebug() << "Error querying data:" << query.lastError().text();
        return false;
    }

    if (query.next()) {
        // 输出查询结果
        qDebug() << "Sensor ID:" << query.value(0).toString();
        qDebug() << "Calibration Code:" << query.value(1).toString();
        qDebug() << "Plant Doctor:" << query.value(2).toString();
        qDebug() << "Treatment Doctor:" << query.value(3).toString();
        qDebug() << "Planting Date:" << query.value(4).toString();
        qDebug() << "Location:" << query.value(5).toString();
        return true;  // 查询成功
    } else {
        qDebug() << "No data found for sensor_id:" << sensorId;
        return false;  // 未找到数据
    }
}

//通过传感器序列号查询位置信息
QString DatabaseManager::getLocationBySensorId(const QString &sensorId)
{
    QSqlQuery query;
    query.prepare(R"(
        SELECT location
          FROM sensor_info
         WHERE sensor_id = :id
         LIMIT 1
    )");
    query.bindValue(":id", sensorId);

    if (!query.exec()) {
        qWarning() << "Location query failed:" << query.lastError().text();
        return {};
    }

    if (!query.next()) {
        qWarning() << "No record for sensor_id =" << sensorId;
        return {};
    }

    // 去掉空格并标准化小写
    return query.value(0).toString().trimmed().toLower();
}
