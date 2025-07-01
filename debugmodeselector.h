#ifndef DEBUGMODESELECTOR_H
#define DEBUGMODESELECTOR_H

#include<QDialog>
#include "CloseOnlyWindow.h"

class DebugModeSelector : public CloseOnlyWindow {
    Q_OBJECT

public:
    explicit    DebugModeSelector(QWidget *parent = nullptr);    //防止隐式类型转换

signals:
    void modeSelected(QString mode);

private:
    float scaleX;
    float scaleY;
};

#endif // DEBUGMODESELECTOR_H
