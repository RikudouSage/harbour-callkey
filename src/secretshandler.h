#ifndef SECRETS_H
#define SECRETS_H

#include <QObject>
#include <QString>

#ifndef QT_DEBUG
#include <Sailfish/Secrets/secretmanager.h>
#include <Sailfish/Secrets/request.h>
#include <Sailfish/Secrets/secret.h>

using Sailfish::Secrets::SecretManager;
using Sailfish::Secrets::Request;
using Sailfish::Secrets::Secret;
#endif

class SecretsHandler : public QObject
{
    Q_OBJECT
public:
    explicit SecretsHandler(QObject *parent = nullptr);

    QString getPassword(const QString &username, const QString &server, const quint16 &port, const QString &target);
    void setPassword(const QString &username, const QString &server, const quint16 &port, const QString &target, const QString &password);

    bool clearAllSecrets();

private:
    static QString passwordKey(const QString &username, const QString &server, const quint16 &port, const QString &target);
    bool storeData(const QString &name, const QString &data);
    QString getData(const QString &name);
    bool deleteSecret(const QString &name);
#ifndef QT_DEBUG
    static const QString collectionName;
    SecretManager* secretManager = new SecretManager(this);
    bool hasCollection = false;

    bool isResultValid(const Request &request);
    bool isSecretValid(const Secret &secret);
    Secret getSecret(const QString &name);
    bool createCollection();
    Secret::Identifier toIdentifier(const QString &name);
#endif
};

#endif // SECRETS_H
