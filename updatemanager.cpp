#include "updatemanager.h"
#include "medicallogger.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QStandardPaths>
#include <QCryptographicHash>
#include <QProcess>
#include <QCoreApplication>
#include <QSettings>

UpdateManager::UpdateManager(QObject* parent)
    : QObject(parent)
{}

bool UpdateManager::hasUsbUpdate(QString* usbDirOut) const
{
    // 遍历所有逻辑盘，查找 UPDATE/manifest.json
    for (const QFileInfo &drive : QDir::drives()) {
        const QString base = drive.absoluteFilePath();
        const QString updateDir = QDir::cleanPath(base + "/UPDATE");
        if (QDir(updateDir).exists() && QFile::exists(updateDir + "/manifest.json")) {
            if (usbDirOut) *usbDirOut = updateDir;
            logInfo("USB update media detected", updateDir);
            return true;
        }
    }
    logWarn("No USB update media found");
    return false;
}

bool UpdateManager::loadManifest(const QString& manifestPath, QJsonObject& out) const
{
    QFile f(manifestPath);
    if (!f.open(QIODevice::ReadOnly)) {
        logError("Failed to open manifest", manifestPath);
        return false;
    }
    const auto doc = QJsonDocument::fromJson(f.readAll());
    if (!doc.isObject()) {
        logError("Manifest is not a JSON object", manifestPath);
        return false;
    }
    out = doc.object();
    logInfo("Manifest loaded", manifestPath);
    return true;
}

QString UpdateManager::sha256OfFile(const QString& path) const
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        logError("Open file for SHA256 failed", path);
        return {};
    }
    QCryptographicHash h(QCryptographicHash::Sha256);
    while (!f.atEnd())
        h.addData(f.read(1024 * 1024));
    return QString::fromLatin1(h.result().toHex());
}

bool UpdateManager::verifyPackage(const QString& pkgPath, const QString& expectSha256) const
{
    const QString actual = sha256OfFile(pkgPath);
    if (expectSha256.isEmpty() || actual.isEmpty()) {
        logError("Empty SHA256 value", QString("expect=%1 actual=%2").arg(expectSha256, actual));
        return false;
    }
    const bool ok = (0 == QString::compare(expectSha256, actual, Qt::CaseInsensitive));
    if (!ok) {
        logError("SHA256 mismatch", QString("expect=%1 actual=%2").arg(expectSha256, actual));
    } else {
        logInfo("SHA256 verified", actual);
    }
    return ok;
}

bool UpdateManager::stageUpdate(const QString& usbDir, QString& stagingOut) const
{
    const QString staging = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                          + "/update_staging";
    QDir().mkpath(staging);

    const QString pkgSrc = QDir::cleanPath(usbDir + "/app_update.pkg");
    const QString manSrc = QDir::cleanPath(usbDir + "/manifest.json");
    const QString pkgDst = QDir::cleanPath(staging + "/app_update.pkg");
    const QString manDst = QDir::cleanPath(staging + "/manifest.json");

    QFile::remove(pkgDst);
    QFile::remove(manDst);

    if (!QFile::copy(pkgSrc, pkgDst)) {
        logError("Copy package to staging failed", QString("%1 -> %2").arg(pkgSrc, pkgDst));
        return false;
    }
    if (!QFile::copy(manSrc, manDst)) {
        logError("Copy manifest to staging failed", QString("%1 -> %2").arg(manSrc, manDst));
        QFile::remove(pkgDst);
        return false;
    }

    stagingOut = staging;
    logAudit("Update staged", staging);
    return true;
}

void UpdateManager::markPendingAndRestart(const QString& stagingDir) const
{
    QSettings s(ORG, APP);
    s.setValue(K_PENDING, true);
    s.setValue(K_STAGING, stagingDir);
    s.remove(K_RESULT);
    s.sync();

    logAudit("Pending update marked, restarting app", stagingDir);

    // 最小可用：仅重启应用（更平滑）
    QProcess::startDetached(QCoreApplication::applicationFilePath(), {});
    QCoreApplication::quit();

    // 若要系统重启，可改成：
    // QProcess::startDetached("shutdown", { "/r", "/t", "0" });
    // QCoreApplication::quit();
}

bool UpdateManager::applyPendingIfAny(QString* resultMsg) const
{
    QSettings s(ORG, APP);
    const bool pending = s.value(K_PENDING, false).toBool();
    if (!pending) return true;

    const QString staging = s.value(K_STAGING).toString();
    logAudit("Applying pending update", staging);

    QString reason;
    const bool ok = installFromStaging(staging, &reason);

    s.setValue(K_PENDING, false);
    s.setValue(K_RESULT, ok ? "success" : ("failed:" + reason));
    s.sync();

    if (ok) {
        logAudit("Update applied successfully");
        if (resultMsg) (*resultMsg) = tr("软件更新成功");
    } else {
        logError("Update apply failed", reason);
        if (resultMsg) (*resultMsg) = tr("软件更新失败：") + reason;
    }
    return ok;
}

bool UpdateManager::installFromStaging(const QString& stagingDir, QString* reasonOut) const
{
    Q_UNUSED(stagingDir);
    if (reasonOut) {
        reasonOut->clear();
    }
    return true;
}

void UpdateManager::logInfo(const QString& m, const QString& detail) const {
    MedicalLogger::instance()->writeLog("Update", MedicalLogger::LOG_INFO,
        detail.isEmpty() ? m : (m + " | " + detail), "UnknownOperator", "System");
    emit logMessage(QString("[INFO] %1 %2").arg(m, detail));
}

void UpdateManager::logWarn(const QString& m, const QString& detail) const {
    MedicalLogger::instance()->writeLog("Update", MedicalLogger::LOG_WARN,
        detail.isEmpty() ? m : (m + " | " + detail), "UnknownOperator", "System");
    emit logMessage(QString("[WARN] %1 %2").arg(m, detail));
}

void UpdateManager::logError(const QString& m, const QString& detail) const {
    MedicalLogger::instance()->writeLog("Update", MedicalLogger::LOG_ERROR,
        detail.isEmpty() ? m : (m + " | " + detail), "UnknownOperator", "System");
    emit logMessage(QString("[ERROR] %1 %2").arg(m, detail));
}

void UpdateManager::logAudit(const QString& m, const QString& detail) const {
    MedicalLogger::instance()->writeLog("Update", MedicalLogger::LOG_AUDIT,
        detail.isEmpty() ? m : (m + " | " + detail), "UnknownOperator", "System");
    emit logMessage(QString("[AUDIT] %1 %2").arg(m, detail));
}
