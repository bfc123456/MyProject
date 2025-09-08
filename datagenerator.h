#ifndef DATAGENERATOR_H
#define DATAGENERATOR_H

#include <QObject>
#include <QTimer>

class DataGenerator : public QObject{
    Q_OBJECT
public:
    explicit DataGenerator(int intervalMs = 50,QObject* parent = nullptr);
public slots:
    void start();
signals:
    void newSample(double value);
private:
    QTimer m_timer;
    double roundTo2(double v);
};

#endif // DATAGENERATOR_H
