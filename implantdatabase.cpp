#include "implantdatabase.h"
#include <QSqlError>
#include <QRegularExpression>
#include <QDebug>
#include <QDate>

ImplantDatabase::ImplantDatabase(const QString& dbPath) {
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbPath);
    if (!db.open()) {
        qDebug() << "Failed to open database:" << db.lastError().text();
    } else {
        qDebug() << "Database opened successfully at" << dbPath;
    }
}

ImplantDatabase::~ImplantDatabase() {
    if (db.isOpen()) {
        db.close();
    }
}

bool ImplantDatabase::isOpen() const {
    return db.isOpen();
}

bool ImplantDatabase::isValidDate(const QString& date, QString& errorMessage) const {
    QRegularExpression re("^\\d{4}/(?:[1-9]|1[0-2])/(?:[1-9]|[12]\\d|3[01])$"); //^匹配字符开始$匹配字符结束
    if (!re.match(date).hasMatch()) {
        errorMessage = "Invalid date format. Must be YYYY/M/D or YYYY/MM/DD.";
        return false;
    }
    QDate qDate = QDate::fromString(date, "yyyy/M/d");
    if (!qDate.isValid()) {
        errorMessage = "Invalid date. Check month and day values.";
        return false;
    }
    return true;
}

bool ImplantDatabase::insertImplant(const QString& sensorSerial, const QString& implantDoctor, const QString& calibrationCode,
                                   const QString& treatmentDoctor, const QString& implantDate, const QString& implantLocation,
                                   QString& errorMessage) {
    if (!isOpen()) {
        errorMessage = "Database is not open.";
        return false;
    }

    if (!isValidDate(implantDate, errorMessage)) {
        return false;
    }

    if (sensorSerial.isEmpty() || implantDoctor.isEmpty() || calibrationCode.isEmpty() ||
        treatmentDoctor.isEmpty() || implantLocation.isEmpty()) {
        errorMessage = "All fields must be non-empty.";
        return false;
    }

    QSqlQuery query(db);
    query.prepare(R"(
        INSERT INTO implants (sensor_serial, implant_doctor, calibration_code, treatment_doctor, implant_date, implant_location)
        VALUES (:sensorSerial, :implantDoctor, :calibrationCode, :treatmentDoctor, :implantDate, :implantLocation)
    )");
    query.bindValue(":sensorSerial", sensorSerial);
    query.bindValue(":implantDoctor", implantDoctor);
    query.bindValue(":calibrationCode", calibrationCode);
    query.bindValue(":treatmentDoctor", treatmentDoctor);
    query.bindValue(":implantDate", implantDate);
    query.bindValue(":implantLocation", implantLocation);

    if (!query.exec()) {
        errorMessage = query.lastError().text();
        return false;
    }

    errorMessage = "Insert successful!";
    return true;
}

bool ImplantDatabase::getAllImplants(QList<QMap<QString, QVariant>>& implants, QString& errorMessage) {
    implants.clear();
    if (!isOpen()) {
        errorMessage = "Database is not open.";
        return false;
    }

    QSqlQuery query(db);
    if (!query.exec("SELECT * FROM implants")) {
        errorMessage = query.lastError().text();
        return false;
    }

    while (query.next()) {
        QMap<QString, QVariant> record;
        record["id"] = query.value("id");
        record["sensor_serial"] = query.value("sensor_serial");
        record["implant_doctor"] = query.value("implant_doctor");
        record["calibration_code"] = query.value("calibration_code");
        record["treatment_doctor"] = query.value("treatment_doctor");
        record["implant_date"] = query.value("implant_date");
        record["implant_location"] = query.value("implant_location");
        record["created_at"] = query.value("created_at");
        implants.append(record);
    }

    errorMessage = implants.isEmpty() ? "No records found." : "Query successful!";
    return true;
}

bool ImplantDatabase::updateImplantLocation(const QString& sensorSerial, const QString& implantLocation, QString& errorMessage) {
    if (!isOpen()) {
        errorMessage = "Database is not open.";
        return false;
    }

    if (sensorSerial.isEmpty()) {
        errorMessage = "Sensor serial cannot be empty.";
        return false;
    }

    if (implantLocation.isEmpty()) {
        errorMessage = "Implant location cannot be empty.";
        return false;
    }

    QSqlQuery query(db);
    query.prepare("UPDATE implants SET implant_location = :implantLocation WHERE sensor_serial = :sensorSerial");
    query.bindValue(":implantLocation", implantLocation);
    query.bindValue(":sensorSerial", sensorSerial);

    if (!query.exec()) {
        errorMessage = query.lastError().text();
        return false;
    }

    if (query.numRowsAffected() == 0) {
        errorMessage = "No record found for sensor_serial: " + sensorSerial;
        return false;
    }

    errorMessage = "Location updated successfully!";
    return true;
}

bool ImplantDatabase::deleteImplant(const QString& sensorSerial, QString& errorMessage) {
    if (!isOpen()) {
        errorMessage = "Database is not open.";
        return false;
    }

    if (sensorSerial.isEmpty()) {
        errorMessage = "Sensor serial cannot be empty.";
        return false;
    }

    QSqlQuery query(db);
    query.prepare("DELETE FROM implants WHERE sensor_serial = :sensorSerial");
    query.bindValue(":sensorSerial", sensorSerial);

    if (!query.exec()) {
        errorMessage = query.lastError().text();
        return false;
    }

    if (query.numRowsAffected() == 0) {
        errorMessage = "No record found for sensor_serial: " + sensorSerial;
        return false;
    }

    errorMessage = "Implant deleted successfully!";
    return true;
}
