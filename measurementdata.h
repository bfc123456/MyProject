
#ifndef MEASUREMENTDATA_H
#define MEASUREMENTDATA_H

#include<QString>
#include<QPointF>
#include <QVector>

/**
 * @struct MeasurementData
 * @brief 用于存储一次完整测量的结果数据
 *
 * MeasurementData 结构体封装了测量过程中采集到的关键信息，
 * 包括传感器标识、时间戳、血压数值、心率以及对应的波形数据点。
 *
 * 主要用途：
 * - 记录一次测量的所有原始与计算结果
 * - 作为数据传输或存储的基本单元
 * - 提供给 UI 展示、数据库保存或后续分析使用
 */

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
