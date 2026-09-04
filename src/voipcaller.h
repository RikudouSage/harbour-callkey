#ifndef VOIPCALLER_H
#define VOIPCALLER_H

#include <QObject>
#include <QStringList>

#include "secretshandler.h"

class VoipCaller : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString sipServer MEMBER sipServer)
    Q_PROPERTY(QString sipUsername MEMBER sipUsername)
    Q_PROPERTY(QString sipPassword READ sipPassword WRITE setSipPassword NOTIFY sipPasswordChanged)
    Q_PROPERTY(quint16 sipServerPort MEMBER sipServerPort)
    Q_PROPERTY(QString sipTransport MEMBER sipTransport)
    Q_PROPERTY(QString advertisedHost MEMBER advertisedHost)
    Q_PROPERTY(QString localAddress MEMBER localAddress)
    Q_PROPERTY(quint16 localPort MEMBER localPort)
    Q_PROPERTY(bool nat MEMBER nat)
    Q_PROPERTY(QString target MEMBER target)
    Q_PROPERTY(quint64 timeoutMs MEMBER timeoutMs)
    Q_PROPERTY(bool ignoreTargetDeclineErrors MEMBER ignoreTargetDeclineErrors)
    Q_PROPERTY(QStringList earlySuccessResponses MEMBER earlySuccessResponses)
public:
    explicit VoipCaller(SecretsHandler *secrets, QObject *parent = nullptr);
    Q_INVOKABLE void placeCall();
    Q_INVOKABLE bool removePassword();

    QString sipPassword();
    void setSipPassword(const QString &value);

signals:
    void callSucceeded();
    void callFailed(const QString &error);
    void sipPasswordChanged();

private:
    QString sipServer;
    QString sipUsername;

    quint16 sipServerPort;
    QString sipTransport;

    QString advertisedHost;
    QString localAddress;
    quint16 localPort;

    bool nat = true;

    QString target;
    quint64 timeoutMs;
    bool ignoreTargetDeclineErrors = true;
    QStringList earlySuccessResponses = {QStringLiteral("2xx"), QStringLiteral("180")};

private:
    SecretsHandler *secrets;
};

#endif // VOIPCALLER_H
