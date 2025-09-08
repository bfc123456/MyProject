#ifndef UPDATEMANAGER_H
#define UPDATEMANAGER_H

#include <QObject>
#include <QJsonObject>
#include <QString>

/**
 * @class UpdateManager
 * @brief 离线软件更新管理器（U 盘更新，最小可用版）
 *
 * 该类负责发现 U 盘中的更新包、校验完整性、复制至本地暂存目录（staging），
 * 写入“挂起更新”标记并触发重启。在应用重新启动后，调用 applyPendingIfAny()
 * 完成安装（本示例提供占位实现，便于后续替换为解包/覆盖/DB 迁移逻辑）。
 *
 * 目录/文件约定（U 盘）：
 *   UPDATE/
 *     app_update.pkg        // 更新包（zip 或自定义包）
 *     manifest.json         // 清单（版本号、sha256、大小等）
 *     signature.sig         // 可选：数字签名（留作扩展）
 *
 * 关键键值（QSettings）：
 *   "update/pending"  : bool    是否有挂起更新
 *   "update/staging"  : QString 暂存目录路径（包含 pkg 与 manifest）
 *   "update/result"   : QString 安装结果（"success"/"failed:<reason>"）
 *
 * 集成建议：
 *  - SettingsWidget::onUpdateClicked() 中使用 hasUsbUpdate/loadManifest/verifyPackage/stageUpdate/markPendingAndRestart
 *  - main() 启动早期调用 applyPendingIfAny() 执行安装
 *  - 全流程使用 MedicalLogger 记录，满足追溯要求
 */
class UpdateManager : public QObject
{
    Q_OBJECT
public:
    explicit UpdateManager(QObject* parent = nullptr);

    // === 发现与校验 ===
    // 查找带 UPDATE/manifest.json 的 U 盘目录，找到则返回 true，并输出 usbDir
    bool hasUsbUpdate(QString* usbDirOut = nullptr) const;

    // 读取 manifest.json 到 out
    bool loadManifest(const QString& manifestPath, QJsonObject& out) const;

    // 计算 pkg 的 SHA-256 并与期望值比对
    bool verifyPackage(const QString& pkgPath, const QString& expectSha256) const;

    // 将 U 盘中的 pkg 和 manifest 复制到本地暂存目录 stagingOut
    bool stageUpdate(const QString& usbDir, QString& stagingOut) const;

    // 写“挂起更新”标记并重启应用（最小可用：仅重启应用；可扩展为系统重启）
    void markPendingAndRestart(const QString& stagingDir) const;

    // 应用启动时调用：若存在挂起更新，则在此执行安装（占位实现，返回 true 视为成功）
    bool applyPendingIfAny(QString* resultMsg = nullptr) const;

signals:
    // 需要显示进度时可外接（当前最小版未使用）
    void progressChanged(int value) const;
    void logMessage(QString msg) const;

private:
    // 计算文件 SHA-256（工具）
    QString sha256OfFile(const QString& path) const;

    // 写日志（统一封装，避免到处 include）
    void logInfo (const QString& m, const QString& detail = {}) const;
    void logWarn (const QString& m, const QString& detail = {}) const;
    void logError(const QString& m, const QString& detail = {}) const;
    void logAudit(const QString& m, const QString& detail = {}) const;

    // 实际安装流程（占位，后续你可替换为解包/替换/DB迁移）
    bool installFromStaging(const QString& stagingDir, QString* reasonOut = nullptr) const;

    // Settings 键命名（集中管理，避免硬编码）
    static constexpr const char* ORG      = "YourCompany";
    static constexpr const char* APP      = "MeasurementApp";
    static constexpr const char* K_PENDING= "update/pending";
    static constexpr const char* K_STAGING= "update/staging";
    static constexpr const char* K_RESULT = "update/result";
};

#endif // UPDATEMANAGER_H
