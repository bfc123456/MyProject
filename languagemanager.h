#ifndef LANGUAGEMANAGER_H
#define LANGUAGEMANAGER_H

#include <QObject>
#include <QTranslator>
#include <QSettings>
#include <QApplication>

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
