#ifndef SERIALSTORE_H
#define SERIALSTORE_H

#pragma once

#include <QString>

/**
 * @brief 单例类，用于存储并全局访问设备序列号
 */
class SerialStore {
public:
    /**
     * @brief 获取唯一实例
     */
    static SerialStore& instance() {
        static SerialStore inst;
        return inst;
    }

    /**
     * @brief 设置设备序列号
     * @param s 新的序列号
     */
    void setSerial(const QString& s) { m_serial = s; }

    /**
     * @brief 获取当前存储的设备序列号
     * @return 序列号字符串
     */
    QString serial() const { return m_serial; }

private:
    SerialStore() = default;
    ~SerialStore() = default;

    // 禁用拷贝与赋值
    SerialStore(const SerialStore&) = delete;
    SerialStore& operator=(const SerialStore&) = delete;

    QString m_serial;
};

#endif // SERIALSTORE_H
