#include "customyscaledraw.h"
#include <QString>
#include <QtGlobal>
#include <QColor>

CustomYScaleDraw::CustomYScaleDraw(int decimalDigits) {
    setDecimalDigits(decimalDigits);
}

QwtText CustomYScaleDraw::label(double value) const {
    // 1. 强制使用十进制格式 'f'
    QString labelText = QString::number(value, 'f', m_decimalDigits);

    // 2. 若设为 0 位小数，移除末尾可能出现的 .0
    if (m_decimalDigits == 0 && labelText.contains('.')) {
        QStringList parts = labelText.split('.');
        if (!parts.isEmpty()) {
            labelText = parts.at(0);
        }
    }

    // 3. 构造白色文字
    QwtText text(labelText);
    text.setColor(Qt::white);
    text.setFont(QFont("Arial", 10));
    return text;
}

void CustomYScaleDraw::setDecimalDigits(int digits) {
    int newDigits = qBound(0, digits, 8);

    if (m_decimalDigits != newDigits) {
        m_decimalDigits = newDigits;

        // 告诉 Qwt：“我的格式变了，下次画刻度时别用旧缓存，重新算一遍”
        invalidateCache();
    }
}
