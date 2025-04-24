#include "databasemanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

DatabaseManager::DatabaseManager(const QString &dbPath)
    : dbPath(dbPath)
{
    db = QSqlDatabase::addDatabase("QSQLITE");  // 使用 SQLite 数据库
    db.setDatabaseName(dbPath);  // 设置数据库文件路径
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
