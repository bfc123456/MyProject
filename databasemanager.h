
#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QString>

class DatabaseManager
{
public:
    DatabaseManager(const QString &dbPath);
    ~DatabaseManager();

    bool openDatabase();  // 打开数据库连接
    bool closeDatabase();  // 关闭数据库连接

    bool insertSensorInfo(const QString &sensorId, const QString &calibrationCode,
                          const QString &plantDoctor, const QString &treatmentDoctor,
                          const QString &plantingDate, const QString &location);  // 插入数据

    bool querySensorInfo(const QString &sensorId);  // 查询数据

    static DatabaseManager& instance(); //获取单例

    QString getLocationBySensorId(const QString &sensorId);

private:
    QSqlDatabase db;
    QString dbPath;
};

#endif // DATABASEMANAGER_H
