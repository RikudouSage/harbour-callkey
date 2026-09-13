#include "callkeydbus.h"

#include <QEventLoop>

#include "defer.h"

CallKeyDBus::CallKeyDBus(Accounts *accounts, SecretsHandler *secrets, QObject *parent)
    : QObject(parent), accounts(accounts), secrets(secrets)
{
}

QStringList CallKeyDBus::listActions()
{
    QStringList result;
    for (const auto &account : accounts->accounts()) {
        result.append(account.toObject().value("name").toString());
    }

    return result;
}

bool CallKeyDBus::trigger(const QString &actionName, bool synchronous)
{
    const auto action = accounts->getAccountByName(actionName);
    if (action.isEmpty()) {
        emit actionFinished(actionName, false, "the action does not exist");
        return false;
    }

    auto caller = new VoipCaller(secrets, this);
    caller->setProperty("sipServer", action.value("sipServer").toString());
    caller->setProperty("sipUsername", action.value("sipUsername").toString());
    caller->setProperty("sipServerPort", action.value("sipServerPort").toInt());
    caller->setProperty("sipTransport", action.value("sipTransport").toString());
    caller->setProperty("advertisedHost", action.value("advertisedHost").toString());
    caller->setProperty("localAddress", action.value("localAddress").toString());
    caller->setProperty("localPort", action.value("localPort").toInt());
    caller->setProperty("nat", action.value("nat").toBool());
    caller->setProperty("target", action.value("target").toString());
    caller->setProperty("timeoutMs", action.value("timeoutMs").toInt());
    caller->setProperty("ignoreTargetDeclineErrors", action.value("ignoreTargetDeclineErrors").toBool());
    caller->setProperty("earlySuccessResponses", action.value("earlySuccessResponses").toArray());

    QString reason;
    bool success = false;
    bool finished = false;

    auto waiter = new QEventLoop(this);
    connect(caller, &VoipCaller::callSucceeded, waiter, [=, &success, &finished] {
        if (!synchronous) {
            emit actionFinished(actionName, true, "");
            caller->deleteLater();
            waiter->deleteLater();
            return;
        }

        success = true;
        finished = true;
        waiter->quit();
    });
    connect(caller, &VoipCaller::callFailed, waiter, [=, &success, &finished, &reason](const QString &error) {
        if (!synchronous) {
            emit actionFinished(actionName, false, error);
            caller->deleteLater();
            waiter->deleteLater();
            return;
        }

        success = false;
        finished = true;
        reason = error;
        waiter->quit();
    });

    caller->placeCall();
    emit actionStarted(actionName);
    if (!synchronous) {
        return true;
    }
    defer({
        caller->deleteLater();
        waiter->deleteLater();
    });

    if (!finished) {
        waiter->exec();
    }

    emit actionFinished(actionName, success, reason);
    return success;
}
