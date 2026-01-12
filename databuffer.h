#ifndef DATABUFFER_H
#define DATABUFFER_H

#include<QQueue>
#include<QMutex>

class DataBuffer{
public:
    QQueue<QByteArray> adcRawDataQueue; //用于存储ADC原始数据包
    QQueue<QByteArray> fftResultQueue;  //用于存储FFT处理结果的数据包
    QMutex mutex; //互斥锁。确保线程安全
};

#endif // DATABUFFER_H
