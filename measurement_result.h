#ifndef MEASUREMENT_RESULT_H
#define MEASUREMENT_RESULT_H

#include <QDateTime>
#include <QString>

struct MeasurementResult {
    float diastolic;    // 舒张压
    float systolic;     // 收缩压
    float average;      // 平均值
    float heartRate;    // 心率
    QDateTime timestamp;// 测量时间戳

    QString toString() const {
        return QString("舒张压: %1, 收缩压: %2, 平均值: %3, 心率: %4, 时间: %5")
                .arg(diastolic, 0, 'f', 2)
                .arg(systolic, 0, 'f', 2)
                .arg(average, 0, 'f', 2)
                .arg(heartRate, 0, 'f', 2)
                .arg(timestamp.toString("yyyy-MM-dd hh:mm:ss"));
    }
};

#endif // MEASUREMENT_RESULT_H
