#include "databasemanager.h"
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QDateTime>

databasemanager::databasemanager() {}

databasemanager::~databasemanager() {}

bool databasemanager::connectDatabase() {
    // 使用SQLite数据库驱动
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("my_database.db");

    // 尝试打开数据库
        if (!db.open()) {
            qDebug() << "Error: Unable to open database!";
            return false;
        }
        qDebug() << "Database connected successfully!";

        // 创建表格，如果表格不存在
        QSqlQuery query;
        QString createTableQuery = R"(
            CREATE TABLE IF NOT EXISTS pulmonary_artery_pressure (
                number INTEGER PRIMARY KEY AUTOINCREMENT,
                diastolicValue REAL,
                systolicValue REAL,
                avgValue REAL,
                heartbeatValue INTEGER,
                upload_time TEXT
            );
        )";

        if (!query.exec(createTableQuery)) {
            qDebug() << "Error: Failed to create table!" << query.lastError().text();
            return false;
        }

        qDebug() << "Table 'pulmonary_artery_pressure' created successfully!";
        return true;
}

bool databasemanager::insertData(double diastolic , double systolic , double avg , int heartbeat ) {
    if (!db.isOpen()) {
        qDebug() << "Database is not open!";
        return false;  // 数据库没有打开
    }

    QSqlQuery query;
    // 插入数据的 SQL 语句
    query.prepare("INSERT INTO pulmonary_artery_pressure (diastolicValue, systolicValue, avgValue, heartbeatValue, upload_time) "
                  "VALUES (:diastolic, :systolic, :avg, :heartbeat, :upload_time)");

    // 绑定参数
    query.bindValue(":diastolic", diastolic);
    query.bindValue(":systolic", systolic);
    query.bindValue(":avg", avg);
    query.bindValue(":heartbeat", heartbeat);
    query.bindValue(":upload_time", QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));

    qDebug() << "SQL query: " << query.executedQuery();
    qDebug() << "Bound values: "
             << "diastolic:" << diastolic
             << "systolic:" << systolic
             << "avg:" << avg
             << "heartbeat:" << heartbeat;

    // 执行查询
    if (query.exec()) {
        qDebug() << "Data inserted successfully.";
    } else {
        qDebug() << "Failed to insert data: " << query.lastError();
        return false;
    }

    return true;
}


// 查询数据
QSqlQuery databasemanager::queryData() {
    QSqlQuery query;
    if (!db.isOpen()) {
        qDebug() << "Database is not open!";
        return query;  // 返回一个无效的查询对象
    }

    query.prepare("SELECT * FROM pulmonary_artery_pressure");

    if (query.exec()) {
        qDebug() << "Query executed successfully111.";
        return query;  // 返回有效的查询对象
    } else {
        qDebug() << "Query failed: " << query.lastError().text();
        return query;  // 返回无效查询对象
    }
}

bool databasemanager::deleteData(int number) {
    if (!db.isOpen()) {
        qDebug() << "Database is not open!";
        return false;  // 数据库没有打开
    }

    QSqlQuery query;
    query.prepare("DELETE FROM pulmonary_artery_pressure WHERE number = :number");
    query.bindValue(":number", number);

    if (query.exec()) {
        qDebug() << "Data deleted successfully.";
    } else {
        qDebug() << "Failed to delete data: " << query.lastError();
        return false;
    }

    return true;
}

void databasemanager::closeDatabase() {
    if (db.isOpen()) {
        db.close();  // 关闭数据库连接
        qDebug() << "Database connection closed.";
    } else {
        qDebug() << "Database is already closed.";
    }
}


