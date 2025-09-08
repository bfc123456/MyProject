
#ifndef LANGUAGEMANAGER_H
#define LANGUAGEMANAGER_H

#include <QObject>
#include <QTranslator>
#include <QSettings>
#include <QApplication>

/**
 * @class LanguageManager
 * @brief 应用多语言管理器
 *
 * 本类用于统一管理应用的语言环境和翻译文件加载，
 * 负责在程序运行过程中完成多语言切换，并通知相关界面刷新文本。
 *
 * 功能职责：
 * - 读取和加载 Qt 翻译文件（.qm）
 * - 根据用户设置或系统环境自动选择语言
 * - 提供接口切换语言（如中文/英文）
 * - 管理 QTranslator 对象的安装与卸载
 * - 在语言切换时发出通知，确保界面即时更新
 *
 * 使用场景：
 * - 应用初始化时，加载用户上次保存的语言偏好
 * - 设置界面中，用户手动切换语言
 * - 支持国际化审查，保证软件符合多语言法规和市场需求
 *
 * @note
 * - 建议在 main.cpp 中全局初始化 LanguageManager；
 * - 结合 QSettings 保存用户语言偏好；
 * - UI 界面应实现 changeEvent(QEvent*)，在语言变更时自动更新界面文字。
 */


class LanguageManager : public QObject
{
    Q_OBJECT

public:
    static LanguageManager& instance();

    // 切换语言
    void switchLanguage(const QString& languageCode);

    // 获取当前语言代码
    QString currentLanguage() const;

signals:
    void languageChanged(const QString& languageCode);  // 广播语言切换信号

private:
    explicit LanguageManager(QObject *parent = nullptr);
    QTranslator translator;
    QString currentLangCode;
};

#endif // LANGUAGEMANAGER_H
