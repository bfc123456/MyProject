#include "languagemanager.h"
#include <QDebug>

LanguageManager::LanguageManager(QObject *parent)
    : QObject(parent)
{
    // 从设置中加载上一次的语言（默认中文）
    QSettings settings("MyCompany", "MyApp");
    currentLangCode = settings.value("language", "zh_CN").toString();

    QString qmPath = ":/translations/translations/" + currentLangCode + ".qm";
    if (translator.load(qmPath)) {
        qApp->installTranslator(&translator);
        qDebug() << "初始化加载语言：" << currentLangCode;
    } else {
        qDebug() << "初始化语言加载失败：" << qmPath;
    }
}

LanguageManager& LanguageManager::instance()
{
    static LanguageManager instance;
    return instance;
}

QString LanguageManager::currentLanguage() const
{
    return currentLangCode;
}

void LanguageManager::switchLanguage(const QString& languageCode)
{
    if (languageCode == currentLangCode)
        return; // 避免重复加载

    qApp->removeTranslator(&translator);

    QString qmPath = ":/translations/translations/" + languageCode + ".qm";
    if (translator.load(qmPath)) {
        qApp->installTranslator(&translator);
        currentLangCode = languageCode;

        // 存储语言设置
        QSettings settings("MyCompany", "MyApp");
        settings.setValue("language", languageCode);

        qDebug() << "语言已切换为：" << languageCode;

        // 发出语言切换信号（供界面刷新）
        emit languageChanged(languageCode);
    } else {
        qDebug() << "语言加载失败：" << qmPath;
    }
}

