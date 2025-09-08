#include "datagenerator.h"
#include <QRandomGenerator>
#include <QtMath>

DataGenerator::DataGenerator(int intervalMs,QObject* parent)
    :QObject(parent)
{
    m_timer.setInterval(intervalMs);    //设置定时器间隔
    connect(&m_timer,&QTimer::timeout,this,[this](){
        //先生成【0,100）的随机数
        double raw = QRandomGenerator::global()->bounded(100.0);
        //四舍五入到两位小数
        double v = roundTo2(raw);
        emit newSample(v);
    });
}

void DataGenerator::start(){
    m_timer.start();
}

double DataGenerator::roundTo2(double v){
    //qRound(V*100)四舍五入到整数，然后除以100.0
    return qRound(v * 100.0)/100.0;
}
