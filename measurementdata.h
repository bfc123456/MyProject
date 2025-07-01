
#ifndef MEASUREMENTDATA_H
#define MEASUREMENTDATA_H

#include<QString>
#include<QPointF>
#include <QVector>

struct MeasurementData {
    int     order;             // 序号
    QString sensorId;          // 传感器号
    QString timestamp;         // 时间戳
    QString sensorSystolic;      //收缩压（最大值）
    QString sensorDiastolic;     //舒张压（最小值）
    QString sensorAvg;      // 平均值
    QString heartRate;  // 心率
    QVector<QPointF>  points;
};

#endif // MEASUREMENTDATA_H
