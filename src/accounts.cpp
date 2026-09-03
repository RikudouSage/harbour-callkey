#include "accounts.h"
#include "defer.h"

#include <QJsonDocument>
#include <QJsonParseError>
#include <QStandardPaths>
#include <QDebug>

constexpr auto dataKey = "data";

namespace {
QString accountKey(const QString &username, const QString &server, const quint16 port, const QString &target)
{
    return QStringLiteral("%1@%2:%3|%4").arg(username).arg(server).arg(port).arg(target);
}
}

Accounts::Accounts(QObject *parent) : QObject(parent)
{
    settings = new QSettings(
        QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/accounts.ini",
        QSettings::IniFormat,
        this
    );
}

QJsonArray Accounts::accounts()
{
    QJsonArray result;
    for (const auto &group : settings->childGroups()) {
        settings->beginGroup(group);
        defer({
            settings->endGroup();
        });

        auto data = settings->value(dataKey).toString();
        QJsonParseError error;
        auto document = QJsonDocument::fromJson(data.toUtf8(), &error);
        if (error.error != QJsonParseError::NoError || !document.isObject()) {
            continue;
        }
        result.append(document.object());
    }

    return result;
}

void Accounts::setAccounts(const QJsonArray &accounts)
{
    if (this->accounts() == accounts) {
        return;
    }

    settings->clear();

    for (const auto &value : accounts) {
        if (!value.isObject()) {
            continue;
        }

        auto account = value.toObject();

        const auto sipUsername = account.value("sipUsername").toString();
        const auto sipServer = account.value("sipServer").toString();
        const auto sipServerPort = account.value("sipServerPort").toInt();
        const auto target = account.value("target").toString();

        if (sipUsername.isEmpty() || sipServer.isEmpty() || sipServerPort <= 0 || sipServerPort > 65535 || target.isEmpty()) {
            continue;
        }

        account.remove("sipPassword");

        const auto group = accountKey(sipUsername, sipServer, static_cast<quint16>(sipServerPort), target);

        settings->beginGroup(group);
        defer({
            settings->endGroup();
        });

        settings->setValue(
            dataKey,
            QString::fromUtf8(
                QJsonDocument(account).toJson(QJsonDocument::Compact)
            )
        );
    }

    settings->sync();
    if (settings->status() != QSettings::NoError) {
        qWarning() << "Failed setting accounts:" << settings->status();
        return;
    }

    emit accountsChanged();
}

QJsonObject Accounts::getAccount(const QString &username, const QString &server, const quint16 &port, const QString &target)
{
    const auto group = accountKey(username, server, port, target);

    settings->beginGroup(group);
    defer({
        settings->endGroup();
    });

    const auto data = settings->value(dataKey).toByteArray();
    if (data.isEmpty()) {
        return {};
    }

    QJsonParseError error;
    const auto document = QJsonDocument::fromJson(data, &error);

    if (error.error != QJsonParseError::NoError || !document.isObject()) {
        return {};
    }

    return document.object();
}

void Accounts::storeAccount(const QJsonObject &account)
{
    const auto sipUsername = account.value("sipUsername").toString();
    const auto sipServer = account.value("sipServer").toString();
    const auto sipServerPort = account.value("sipServerPort").toInt();
    const auto target = account.value("target").toString();

    if (sipUsername.isEmpty() || sipServer.isEmpty() || sipServerPort <= 0 || sipServerPort > 65535 || target.isEmpty()) {
        return;
    }

    auto storedAccount = account;
    storedAccount.remove("sipPassword");

    const auto port = static_cast<quint16>(sipServerPort);

    if (getAccount(sipUsername, sipServer, port, target) == storedAccount) {
        return;
    }

    const auto group = accountKey(sipUsername, sipServer, port, target);

    settings->beginGroup(group);
    defer({
        settings->endGroup();
    });

    settings->setValue(
        dataKey,
        QJsonDocument(storedAccount).toJson(QJsonDocument::Compact)
    );

    settings->sync();
    if (settings->status() != QSettings::NoError) {
        qWarning() << "Failed storing account:" << settings->status();
        return;
    }

    emit accountsChanged();
}

void Accounts::removeAccount(const QJsonObject &account)
{
    const auto sipUsername = account.value("sipUsername").toString();
    const auto sipServer = account.value("sipServer").toString();
    const auto sipServerPort = account.value("sipServerPort").toInt();
    const auto target = account.value("target").toString();

    if (sipUsername.isEmpty() || sipServer.isEmpty() || sipServerPort <= 0 || sipServerPort > 65535 || target.isEmpty()) {
        return;
    }

    const auto group = accountKey(sipUsername, sipServer, static_cast<quint16>(sipServerPort), target);

    settings->remove(group);
    settings->sync();
    if (settings->status() != QSettings::NoError) {
        qWarning() << "Failed removing account:" << settings->status();
        return;
    }

    emit accountsChanged();
}
