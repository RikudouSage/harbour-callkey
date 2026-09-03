#include "secretshandler.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QSettings>

#ifndef QT_DEBUG
#include <Sailfish/Secrets/collectionnamesrequest.h>
#include <Sailfish/Secrets/createcollectionrequest.h>
#include <Sailfish/Secrets/result.h>
#include <Sailfish/Secrets/secret.h>
#include <Sailfish/Secrets/storesecretrequest.h>
#include <Sailfish/Secrets/storedsecretrequest.h>
#include <Sailfish/Secrets/deletesecretrequest.h>
#include <Sailfish/Secrets/deletecollectionrequest.h>

using Sailfish::Secrets::CollectionNamesRequest;
using Sailfish::Secrets::SecretManager;
using Sailfish::Secrets::Request;
using Sailfish::Secrets::Result;
using Sailfish::Secrets::CreateCollectionRequest;
using Sailfish::Secrets::Secret;
using Sailfish::Secrets::StoreSecretRequest;
using Sailfish::Secrets::StoredSecretRequest;
using Sailfish::Secrets::DeleteSecretRequest;
using Sailfish::Secrets::DeleteCollectionRequest;

const QString SecretsHandler::collectionName(QStringLiteral("harbour-callkey"));
#endif

#ifdef QT_DEBUG
static QSettings &insecureEmulatorSecrets()
{
    static QSettings settings(QStringLiteral("cz.chrastecky"), QStringLiteral("cookbook-insecure-emulator-secrets"));
    return settings;
}
#endif

SecretsHandler::SecretsHandler(QObject *parent) : QObject(parent)
{
#ifdef QT_DEBUG
    qWarning() << "Using insecure debug secrets storage. Do not enable this in release builds.";
#else
    CollectionNamesRequest cnr;
    cnr.setManager(secretManager);
    cnr.setStoragePluginName(SecretManager::DefaultEncryptedStoragePluginName);
    cnr.startRequest();
    cnr.waitForFinished();

    hasCollection = isResultValid(cnr) && cnr.collectionNames().contains(collectionName);
#endif
}

QString SecretsHandler::getPassword(const QString &server, const quint16 &port)
{
    return getData("password:" + server + ":" + port);
}

void SecretsHandler::setPassword(const QString &server, const quint16 &port, const QString &password)
{
    storeData("password:" + server + ":" + port, password);
}

bool SecretsHandler::clearAllSecrets()
{
#ifdef QT_DEBUG
    auto &settings = insecureEmulatorSecrets();
    settings.clear();
    settings.sync();
    return settings.status() == QSettings::NoError;
#else
    DeleteCollectionRequest dcr;
    dcr.setCollectionName(collectionName);
    dcr.setStoragePluginName(SecretManager::DefaultEncryptedStoragePluginName);
    dcr.setUserInteractionMode(SecretManager::SystemInteraction);
    dcr.setManager(secretManager);
    dcr.startRequest();
    dcr.waitForFinished();

    auto success = isResultValid(dcr);

    hasCollection = !success;
    return success;
#endif
}

#ifndef QT_DEBUG
bool SecretsHandler::isResultValid(const Request &request)
{
    auto result = request.result();
    auto isSuccess = result.errorCode() == Result::NoError;
    if (!isSuccess) {
        qWarning() << result.errorMessage();
    }

    return isSuccess;
}

bool SecretsHandler::isSecretValid(const Secret &secret)
{
    return !secret.name().isNull() && !secret.name().isEmpty();
}
#endif

bool SecretsHandler::storeData(const QString &name, const QString &data)
{
#ifdef QT_DEBUG
    auto &settings = insecureEmulatorSecrets();
    settings.setValue(name, data);
    settings.sync();
    return settings.status() == QSettings::NoError;
#else
    if (!hasCollection) {
        createCollection();
        // todo handle case where collection isn't created
    }

    auto existingSecret = getSecret(name);
    if (isSecretValid(existingSecret)) {
        deleteSecret(name);
    }

    Secret secret(toIdentifier(name));
    secret.setData(data.toUtf8());

    StoreSecretRequest ssr;
    ssr.setManager(secretManager);
    ssr.setSecretStorageType(StoreSecretRequest::CollectionSecret);
    ssr.setUserInteractionMode(SecretManager::SystemInteraction);
    ssr.setSecret(secret);
    ssr.startRequest();
    ssr.waitForFinished();

    return isResultValid(ssr);
#endif
}

#ifndef QT_DEBUG
Secret SecretsHandler::getSecret(const QString &name)
{
    if (!hasCollection) {
        return Secret();
    }

    StoredSecretRequest ssr;
    ssr.setManager(secretManager);
    ssr.setUserInteractionMode(SecretManager::SystemInteraction);
    ssr.setIdentifier(toIdentifier(name));
    ssr.startRequest();
    ssr.waitForFinished();

    auto success = isResultValid(ssr);
    if (!success) {
        return Secret();
    }

    return ssr.secret();
}
#endif

bool SecretsHandler::deleteSecret(const QString &name)
{
#ifdef QT_DEBUG
    auto &settings = insecureEmulatorSecrets();
    settings.remove(name);
    settings.sync();
    return settings.status() == QSettings::NoError;
#else
    DeleteSecretRequest dsr;
    dsr.setManager(secretManager);
    dsr.setIdentifier(toIdentifier(name));
    dsr.setUserInteractionMode(SecretManager::SystemInteraction);
    dsr.startRequest();
    dsr.waitForFinished();

    return isResultValid(dsr);
#endif
}

QString SecretsHandler::getData(const QString &name)
{
#ifdef QT_DEBUG
    return insecureEmulatorSecrets().value(name).toString();
#else
    auto secret = getSecret(name);
    if (!isSecretValid(secret)) {
        return QString();
    }

    return QString::fromUtf8(secret.data());
#endif
}

#ifndef QT_DEBUG
bool SecretsHandler::createCollection()
{
    CreateCollectionRequest ccr;
    ccr.setManager(secretManager);
    ccr.setCollectionName(collectionName);
    ccr.setAccessControlMode(SecretManager::OwnerOnlyMode);
    ccr.setCollectionLockType(CreateCollectionRequest::DeviceLock);
    ccr.setDeviceLockUnlockSemantic(SecretManager::DeviceLockKeepUnlocked);
    ccr.setStoragePluginName(SecretManager::DefaultEncryptedStoragePluginName);
    ccr.setEncryptionPluginName(SecretManager::DefaultEncryptedStoragePluginName);
    ccr.startRequest();
    ccr.waitForFinished();

    auto success = isResultValid(ccr);
    hasCollection = success;

    return success;
}

SecretHandler::Identifier Secrets::toIdentifier(const QString &name)
{
    return Secret::Identifier(name, collectionName, SecretManager::DefaultEncryptedStoragePluginName);
}
#endif
