// 1) Project Headers
#include "customyscaledraw.h"
// 2)包含小数优化所需的头文件
#include <cmath>
// 3)Qt Headers
#include <QString>

/**
 * @brief 构造函数实现：初始化小数位数
 */
CustomYScaleDraw::CustomYScaleDraw(int decimalDigits)
    : m_decimalDigits(decimalDigits)
{
    // 初始化时确保小数位数在合理范围（0~6）
    setDecimalDigits(decimalDigits);
}

/**
 * @brief 重写 label 方法：强制十进制格式
 */
QwtText CustomYScaleDraw::label(double value) const
{
    // 核心逻辑：用 QString::number 强制十进制，'f' 表示固定小数位格式
    QString labelText = QString::number(value, 'f', m_decimalDigits);

    // 可选优化：若小数位数为 0，移除末尾多余的 ".0"（如 100.0 → 100）
    if (m_decimalDigits == 0)
    {
        labelText.remove(".0");
    }

    // 将 QString 包装为 QwtText 并返回（Qwt 轴标签需要 QwtText 类型）
    return QwtText(labelText);
}

/**
 * @brief 设置小数位数：限制范围为 0~6，避免不合理值
 */
void CustomYScaleDraw::setDecimalDigits(int digits)
{
    // qBound：将 digits 钳位在 0~6 之间（小于 0 取 0，大于 6 取 6）
    m_decimalDigits = qBound(0, digits, 6);
}
