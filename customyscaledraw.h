#ifndef CUSTOMYSCALEDRAW_H
#define CUSTOMYSCALEDRAW_H

// 包含依赖的 Qwt 头文件
#include <qwt_scale_draw.h>
#include <qwt_text.h>
#include <QString>

/**
 * @brief 自定义 Y 轴 ScaleDraw 子类
 * 功能：强制 Y 轴刻度标签以十进制显示，避免科学计数法，支持自定义小数位数
 */
class CustomYScaleDraw : public QwtScaleDraw
{
public:
    /**
     * @brief 构造函数
     * @param decimalDigits 保留的小数位数（默认 0，即整数显示）
     */
    explicit CustomYScaleDraw(int decimalDigits = 0);

    /**
     * @brief 重写父类方法：生成自定义格式的刻度标签
     * @param value 刻度值（double 类型）
     * @return 格式化后的标签文本（QwtText）
     */
    QwtText label(double value) const override;

    /**
     * @brief 设置 Y 轴标签的小数位数
     * @param digits 小数位数（范围：0~6，超出则自动钳位）
     */
    void setDecimalDigits(int digits);

private:
    int m_decimalDigits; // 私有成员：存储保留的小数位数
};

#endif // CUSTOMYSCALEDRAW_H
