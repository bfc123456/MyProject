
#ifndef MEASUREMENTDATA_H
#define MEASUREMENTDATA_H

#include<QString>
#include<QPointF>
#include <QVector>

struct MeasurementData {
    int     order;             // 序号
    QString sensorSystolic;
    QString sensorDiastolic;
    QString sensorAvg;
    QString heartRate;
    QString refSystolic;
    QString refDiastolic;
    QString refAvg;
    QString angle;
    QString note;
    QVector<QPointF>  points;
};

#endif // MEASUREMENTDATA_H
