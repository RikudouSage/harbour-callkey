#include "accounts.h"
#include "defer.h"

#include <QJsonDocument>
#include <QJsonParseError>

constexpr auto dataKey = "data";

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

        const auto sipServer = account.value("sipServer").toString();
        const auto sipServerPort = account.value("sipServerPort").toInt();

        if (sipServer.isEmpty() || sipServerPort <= 0) {
            continue;
        }

        account.remove("sipPassword");

        const auto group = QStringLiteral("%1:%2").arg(sipServer).arg(sipServerPort);

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

QJsonObject Accounts::getAccount(const QString &server, const quint16 &port)
{
    const auto group = QStringLiteral("%1:%2").arg(server).arg(port);

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
    const auto sipServer = account.value("sipServer").toString();
    const auto sipServerPort = account.value("sipServerPort").toInt();

    if (sipServer.isEmpty() || sipServerPort <= 0 || sipServerPort > 65535) {
        return;
    }

    auto storedAccount = account;
    storedAccount.remove("sipPassword");

    const auto port = static_cast<quint16>(sipServerPort);

    if (getAccount(sipServer, port) == storedAccount) {
        return;
    }

    const auto group = QStringLiteral("%1:%2").arg(sipServer).arg(port);

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
