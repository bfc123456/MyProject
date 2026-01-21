#ifndef CUSTOMYSCALEDRAW_H
#define CUSTOMYSCALEDRAW_H

#include <qwt_scale_draw.h>
#include <qwt_text.h>

class CustomYScaleDraw : public QwtScaleDraw {
public:
    explicit CustomYScaleDraw(int decimalDigits = 0);

    // 重写 label 函数以控制格式和颜色
    virtual QwtText label(double value) const override;

    void setDecimalDigits(int digits);

private:
    int m_decimalDigits;
};

#endif // CUSTOMYSCALEDRAW_H
