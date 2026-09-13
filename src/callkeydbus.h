#ifndef CALLKEYDBUS_H
#define CALLKEYDBUS_H

#include <QObject>

#include "accounts.h"
#include "voipcaller.h"
#include "secretshandler.h"

class CallKeyDBus : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "dev.rikudou.callkey.v1")
public:
    explicit CallKeyDBus(Accounts *accounts, SecretsHandler *secrets, QObject *parent = nullptr);

public slots:
    QStringList listActions();
    bool trigger(const QString &actionName, bool synchronous = false);

signals:
    void actionStarted(const QString &actionName);
    void actionFinished(const QString &actionName, bool success, const QString &message);

private:
    Accounts *accounts;
    SecretsHandler *secrets;
};

#endif // CALLKEYDBUS_H
