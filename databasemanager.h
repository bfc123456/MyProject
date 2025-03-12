#ifndef databasemanager_H
#define databasemanager_H

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QDebug>

class databasemanager
{
public:
    databasemanager();
    ~databasemanager();

    QSqlDatabase getDatabase() {
        return db;
    }


    // 连接数据库
    bool connectDatabase();

    // 插入数据
    bool insertData(double diastolic , double systolic , double avg , int heartbeat );

    // 查询数据
    QSqlQuery queryData();

    // 删除数据
    bool deleteData(int id);  // 删除数据的方法，传入id作为参数

    // 关闭数据库
    void closeDatabase();

private:
    QSqlDatabase db;  // 数据库连接
};

#endif // databasemanager_H
