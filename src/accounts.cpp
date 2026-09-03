#include "accounts.h"
#include "defer.h"

#include <QJsonDocument>
#include <QJsonParseError>

constexpr auto dataKey = "data";

namespace {
QString accountKey(const QString &username, const QString &server, const quint16 port)
{
    return QStringLiteral("%1@%2:%3").arg(username).arg(server).arg(port);
}
}

Accounts::Accounts(QObject *parent) : QObject(parent)
{
    settings = new QSettings("cz.chrastecky", "harbour-callkey", this);
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

        if (sipUsername.isEmpty() || sipServer.isEmpty() || sipServerPort <= 0 || sipServerPort > 65535) {
            continue;
        }

        account.remove("sipPassword");

        const auto group = accountKey(sipUsername, sipServer, static_cast<quint16>(sipServerPort));

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
    emit accountsChanged();
}

QJsonObject Accounts::getAccount(const QString &username, const QString &server, const quint16 &port)
{
    const auto group = accountKey(username, server, port);

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

    if (sipUsername.isEmpty() || sipServer.isEmpty() || sipServerPort <= 0 || sipServerPort > 65535) {
        return;
    }

    auto storedAccount = account;
    storedAccount.remove("sipPassword");

    const auto port = static_cast<quint16>(sipServerPort);

    if (getAccount(sipUsername, sipServer, port) == storedAccount) {
        return;
    }

    const auto group = accountKey(sipUsername, sipServer, port);

    settings->beginGroup(group);
    defer({
        settings->endGroup();
    });

    settings->setValue(
        dataKey,
        QJsonDocument(storedAccount).toJson(QJsonDocument::Compact)
    );

    settings->sync();

    emit accountsChanged();
}
