#include "languagemanager.h"
#include <QDebug>

LanguageManager& LanguageManager::instance()
{
    static LanguageManager instance;
    return instance;
}

LanguageManager::LanguageManager(QObject *parent)
    : QObject(parent), translator(new QTranslator(this))
{
    // 从设置中加载上一次的语言（默认中文）
    QSettings settings("MyCompany", "MyApp");
    currentLangCode = settings.value("language", "zh_CN").toString();

    QString qmPath = ":/translations/translations/" + currentLangCode + ".qm";
    if (translator->load(qmPath)) {
        qApp->installTranslator(translator);
        qDebug() << "初始化加载语言：" << currentLangCode;
    } else {
        qDebug() << "初始化语言加载失败：" << qmPath;
    }
}

// 析构函数中无需手动 delete（父对象会处理），但可显式卸载翻译器
LanguageManager::~LanguageManager()
{
    if (translator) {
        qApp->removeTranslator(translator);
    }
}


QString LanguageManager::currentLanguage() const
{
    return currentLangCode;
}

// 切换语言：卸载旧翻译器 -> 加载新翻译器 -> 通知所有窗口更新
void LanguageManager::switchLanguage(const QString& languageCode)
{
    // 避免重复切换相同语言
    if (languageCode == currentLangCode) {
        qDebug() << "LanguageManager: 已处于目标语言，无需切换 -" << languageCode;
        return;
    }

    // 卸载当前翻译器
    qApp->removeTranslator(translator);

    // 加载新语言的翻译文件
    QString qmPath = ":/translations/translations/" + languageCode + ".qm";
    if (translator->load(qmPath)) {
        // 加载成功：安装新翻译器并更新状态
        qApp->installTranslator(translator);
        currentLangCode = languageCode;

        // 保存语言设置到配置文件
        QSettings settings("MyCompany", "MyApp");
        settings.setValue("language", languageCode);

        qDebug() << "LanguageManager: 语言切换成功 -" << languageCode;

        // 发出信号，通知所有窗口更新界面文本
        emit languageChanged(languageCode);
    } else {
        // 加载失败：恢复原语言（可选逻辑，避免程序无翻译）
        qDebug() << "LanguageManager: 语言切换失败，恢复原语言 - 失败路径:" << qmPath;
        // 重新加载原语言（如果需要）
        QString originalQmPath = ":/translations/translations/" + currentLangCode + ".qm";
        if (translator->load(originalQmPath)) {
            qApp->installTranslator(translator);
        }
    }
}
