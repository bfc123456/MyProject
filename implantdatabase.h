#ifndef IMPLANTDATABASE_H
#define IMPLANTDATABASE_H

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QString>
#include <QList>
#include <QMap>
#include <QVariant>

class ImplantDatabase {
public:
    explicit ImplantDatabase(const QString& dbPath);
    ~ImplantDatabase();

    bool isOpen() const;

    bool insertImplant(const QString& sensorSerial, const QString& implantDoctor, const QString& calibrationCode,
                       const QString& treatmentDoctor, const QString& implantDate, const QString& implantLocation,
                       QString& errorMessage);

    bool getAllImplants(QList<QMap<QString, QVariant>>& implants, QString& errorMessage);

    bool updateImplantLocation(const QString& sensorSerial, const QString& implantLocation, QString& errorMessage);

    bool deleteImplant(const QString& sensorSerial, QString& errorMessage);

private:
    bool isValidDate(const QString& date, QString& errorMessage) const;

    QSqlDatabase db;
};

#endif // IMPLANTDATABASE_H
