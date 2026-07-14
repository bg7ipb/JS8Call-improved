

/** \file
 * @brief member function of the UI_Constructor class
 *  queries the GitHub release assets, trigged from the help menu to determine
 *  if the software version is the latest
 */

#include "JS8_UI/mainwindow.h"
#include "JS8_I18N/ILC_runtime.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QCryptographicHash>
#include <QSaveFile>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QMessageBox>
#include <functional>

// --- step4 (W1-b): download + sha256-verify + atomic stage of updated ILC codebook ---
// Fire-and-forget: fetch the codebook artifact, verify its SHA-256 against the
// manifest-declared digest, and (only on match) atomically write it to the user-writable
// staging location. No UI prompt / no hot-reload here — surfacing + applying is step5.
static void downloadAndVerifyCodebook(QObject *parent,
                                      const QString &urlStr,
                                      const QString &expectedSha256,
                                      const QString &destPath,
                                      std::function<void()> onSuccess = nullptr,
                                      std::function<void(const QString &)> onError = nullptr)
{
    const QUrl url(urlStr);
    if (urlStr.isEmpty() || !url.isValid()) {
        qCWarning(mainwindow_js8) << "ILC codebook download: invalid/empty URL:" << urlStr;
        if (onError) onError(QStringLiteral("Invalid codebook update URL."));
        return;
    }

    auto *nam = new QNetworkAccessManager(parent);
    QObject::connect(nam, &QNetworkAccessManager::finished, parent,
                     [nam, expectedSha256, destPath, onSuccess, onError](QNetworkReply *reply) {
        reply->deleteLater();
        nam->deleteLater();

        if (reply->error() != QNetworkReply::NoError) {
            qCWarning(mainwindow_js8) << "ILC codebook download failed:" << reply->errorString();
            if (onError) onError(QStringLiteral("Could not download the codebook (network error)."));
            return;
        }

        const QByteArray payload = reply->readAll();
        const QByteArray actual =
            QCryptographicHash::hash(payload, QCryptographicHash::Sha256).toHex();
        if (actual.compare(expectedSha256.toLatin1(), Qt::CaseInsensitive) != 0) {
            qCWarning(mainwindow_js8) << "ILC codebook sha256 mismatch; expected"
                                      << expectedSha256 << "got" << QString::fromLatin1(actual)
                                      << "- discarding download";
            if (onError) onError(QStringLiteral("Codebook verification failed (checksum mismatch)."));
            return;  // never write a corrupt / mismatched artifact
        }

        QDir().mkpath(QFileInfo(destPath).absolutePath());
        QSaveFile out(destPath);
        if (!out.open(QIODevice::WriteOnly)) {
            qCWarning(mainwindow_js8) << "ILC codebook: cannot open staging file"
                                      << destPath << out.errorString();
            if (onError) onError(QStringLiteral("Could not write the codebook file to disk."));
            return;
        }
        out.write(payload);
        if (!out.commit()) {
            qCWarning(mainwindow_js8) << "ILC codebook: commit failed"
                                      << destPath << out.errorString();
            if (onError) onError(QStringLiteral("Could not save the updated codebook."));
            return;
        }
        qCDebug(mainwindow_js8) << "ILC codebook staged (" << payload.size()
                                << "bytes, sha256 verified) ->" << destPath;
        if (onSuccess) onSuccess();
    });
    nam->get(QNetworkRequest(url));
}

void UI_Constructor::checkVersion(bool const alertOnUpToDate) {
    // --- App manifest check (JSON; independent of codebook check below) ---
    QString updateUrl = QStringLiteral(JS8CALL_UPDATE_URL);
    if (updateUrl.isEmpty()) {
        qCDebug(mainwindow_js8) << "Update URL empty, skipping app check";
    } else {
        auto m = new QNetworkAccessManager(this);
        connect(m, &QNetworkAccessManager::finished, this,
                [this, alertOnUpToDate](QNetworkReply *reply) {
                    if (reply->error()) {
                        qCDebug(mainwindow_js8) << "App check error:"
                                                << reply->errorString();
                        if (alertOnUpToDate) {
                            SelfDestructMessageBox *m =
                                new SelfDestructMessageBox(
                                    60, "检查更新",
                                    QString("无法检查更新：%1")
                                        .arg(reply->errorString()),
                                    QMessageBox::Warning, QMessageBox::Ok,
                                    QMessageBox::Ok, false, this);
                            m->show();
                        }
                        return;
                    }

                    QJsonParseError jErr;
                    QJsonDocument const doc =
                        QJsonDocument::fromJson(reply->readAll(), &jErr);
                    if (jErr.error != QJsonParseError::NoError
                        || !doc.isObject()) {
                        qCDebug(mainwindow_js8)
                            << "App manifest parse error:" << jErr.errorString();
                        if (alertOnUpToDate) {
                            SelfDestructMessageBox *m =
                                new SelfDestructMessageBox(
                                    60, "检查更新",
                                    QString("无法检查更新：服务器响应格式错误（%1）")
                                        .arg(jErr.errorString()),
                                    QMessageBox::Warning, QMessageBox::Ok,
                                    QMessageBox::Ok, false, this);
                            m->show();
                        }
                        return;
                    }
                    QJsonObject const obj = doc.object();
                    QString const appVer = obj.value("version").toString();
                    QString const appUrl = obj.value("url").toString();
                    QString const appLog = obj.value("changelog").toString();
                    if (appVer.isEmpty()) {
                        qCDebug(mainwindow_js8)
                            << "App manifest missing version, skipping";
                        if (alertOnUpToDate) {
                            SelfDestructMessageBox *m =
                                new SelfDestructMessageBox(
                                    60, "检查更新",
                                    "无法检查更新：服务器未提供版本信息",
                                    QMessageBox::Warning, QMessageBox::Ok,
                                    QMessageBox::Ok, false, this);
                            m->show();
                        }
                        return;
                    }

                    auto const currentVersion =
                        QVersionNumber::fromString(version());
                    auto const networkVersion =
                        QVersionNumber::fromString(appVer);

                    qCDebug(mainwindow_js8) << "Checking Version"
                                            << currentVersion << "with"
                                            << networkVersion;

                    if (currentVersion < networkVersion) {

                        QString body =
                            QString("发现新版本 %1。请到 <a href='%2'>GitHub Releases</a> 下载。")
                                .arg(appVer,
                                     appUrl.isEmpty()
                                         ? QStringLiteral(
                                               "https://github.com/bg7ipb/"
                                               "JS8Call-improved/releases")
                                         : appUrl);
                        if (!appLog.isEmpty()) {
                            body += QStringLiteral("<br><br>") + appLog;
                        }

                        SelfDestructMessageBox *m = new SelfDestructMessageBox(
                            60, "检查更新", body,
                            QMessageBox::Information, QMessageBox::Ok,
                            QMessageBox::Ok, false, this);

                        m->show();

                    } else if (alertOnUpToDate) {

                        SelfDestructMessageBox *m = new SelfDestructMessageBox(
                            60, "检查更新",
                            QString("你现在是 %1 版本，已经是最新版了。")
                                .arg(version()),
                            QMessageBox::Information, QMessageBox::Ok,
                            QMessageBox::Ok, false, this);

                        m->show();
                    }
                });

        qCDebug(mainwindow_js8) << "Checking for Updates...";
        QUrl const url(updateUrl);
        QNetworkRequest const r(url);
        m->get(r);
    }

    // --- Codebook manifest check (parse + log only; compare/download later) ---
    QString codebookUrl = QStringLiteral(JS8CALL_CODEBOOK_URL);
    if (codebookUrl.isEmpty()) {
        qCDebug(mainwindow_js8) << "Codebook URL empty, skipping codebook check";
    } else {
        auto m = new QNetworkAccessManager(this);
        connect(m, &QNetworkAccessManager::finished, this,
                [this](QNetworkReply *reply) {
                    if (reply->error()) {
                        qCDebug(mainwindow_js8) << "Codebook check error:"
                                                << reply->errorString();
                        return;
                    }

                    QJsonParseError jErr;
                    QJsonDocument const doc =
                        QJsonDocument::fromJson(reply->readAll(), &jErr);
                    if (jErr.error != QJsonParseError::NoError
                        || !doc.isObject()) {
                        qCDebug(mainwindow_js8)
                            << "Codebook manifest parse error:"
                            << jErr.errorString();
                        return;
                    }
                    QJsonObject const obj = doc.object();
                    const QString remoteCbVer = obj.value("version").toString();
                    const QString cbUrl       = obj.value("url").toString();
                    const QString cbSha256    = obj.value("sha256").toString();
                    const QString minAppVer   = obj.value("min_app_version").toString();
                    qCDebug(mainwindow_js8)
                        << "Codebook manifest parsed: version=" << remoteCbVer
                        << "url=" << cbUrl
                        << "sha256=" << cbSha256
                        << "min_app_version=" << minAppVer;

                    // --- Codebook version comparison (step-3b) ---
                    const QString installedCbVer = ILCRuntime::codebookVersion();
                    const QVersionNumber remoteVN    = QVersionNumber::fromString(remoteCbVer);
                    const QVersionNumber installedVN = QVersionNumber::fromString(installedCbVer);

                    if (installedCbVer.isEmpty() || remoteVN.isNull()) {
                        // dev build (no codebook loaded) or unparseable remote
                        // version -> silently skip codebook update prompt
                    } else {
                        const QVersionNumber appVN    = QVersionNumber::fromString(version());
                        const QVersionNumber minAppVN = QVersionNumber::fromString(minAppVer);
                        if (minAppVN.isNull() || appVN >= minAppVN) {
                            if (remoteVN > installedVN) {
                                qCDebug(mainwindow_js8)
                                    << "Codebook update available:" << remoteCbVer
                                    << "(installed" << installedCbVer << ")";
                                const QString codebookDest =
                                    QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation)
                                    + QStringLiteral("/codebook_cn.csv");
                                if (QMessageBox::question(
                                        this,
                                        "Codebook Update Available",
                                        QString("A new Chinese codebook (version %1) is "
                                                "available. Download it now?")
                                            .arg(remoteVN.toString()))
                                    == QMessageBox::Yes) {
                                    downloadAndVerifyCodebook(this, cbUrl, cbSha256, codebookDest,
                                        /*onSuccess*/ [this]() {
                                            SelfDestructMessageBox *m = new SelfDestructMessageBox(
                                                60, "Codebook Updated",
                                                "The Chinese codebook has been updated. "
                                                "Please restart JS8Call-CN to apply the new codebook.",
                                                QMessageBox::Information, QMessageBox::Ok,
                                                QMessageBox::Ok, false, this);
                                            m->show();
                                        },
                                        /*onError*/ [this](const QString &reason) {
                                            SelfDestructMessageBox *m = new SelfDestructMessageBox(
                                                60, "Codebook Update Failed",
                                                reason,
                                                QMessageBox::Warning, QMessageBox::Ok,
                                                QMessageBox::Ok, false, this);
                                            m->show();
                                        });
                                }
                            }
                        }
                    }
                });

        qCDebug(mainwindow_js8) << "Checking for Codebook Updates...";
        QUrl const curl(codebookUrl);
        QNetworkRequest const cr(curl);
        m->get(cr);
    }
}
