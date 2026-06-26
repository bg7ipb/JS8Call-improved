

/** \file
 * @brief member function of the UI_Constructor class
 *  queries the GitHub release assets, trigged from the help menu to determine
 *  if the software version is the latest
 */

#include "JS8_UI/mainwindow.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

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
                        return;
                    }

                    QJsonParseError jErr;
                    QJsonDocument const doc =
                        QJsonDocument::fromJson(reply->readAll(), &jErr);
                    if (jErr.error != QJsonParseError::NoError
                        || !doc.isObject()) {
                        qCDebug(mainwindow_js8)
                            << "App manifest parse error:" << jErr.errorString();
                        return;
                    }
                    QJsonObject const obj = doc.object();
                    QString const appVer = obj.value("version").toString();
                    QString const appUrl = obj.value("url").toString();
                    QString const appLog = obj.value("changelog").toString();
                    if (appVer.isEmpty()) {
                        qCDebug(mainwindow_js8)
                            << "App manifest missing version, skipping";
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
                            QString("A new version (%1) of JS8Call is now "
                                    "available. Please see the <a "
                                    "href='%2'>GitHub Releases</a> for more "
                                    "details.")
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
                            60, "New Updates Available", body,
                            QMessageBox::Information, QMessageBox::Ok,
                            QMessageBox::Ok, false, this);

                        m->show();

                    } else if (alertOnUpToDate) {

                        SelfDestructMessageBox *m = new SelfDestructMessageBox(
                            60, "No Updates Available",
                            QString("Your version (%1) of JS8Call is "
                                    "up-to-date.")
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
                    qCDebug(mainwindow_js8)
                        << "Codebook manifest parsed: version="
                        << obj.value("version").toString()
                        << "url=" << obj.value("url").toString()
                        << "sha256=" << obj.value("sha256").toString()
                        << "min_app_version="
                        << obj.value("min_app_version").toString();
                });

        qCDebug(mainwindow_js8) << "Checking for Codebook Updates...";
        QUrl const curl(codebookUrl);
        QNetworkRequest const cr(curl);
        m->get(cr);
    }
}
