#include "voipcaller.h"

#include "voip/libvoipringer.h"
#include <QtConcurrent>
#include <QDebug>

VoipCaller::VoipCaller(SecretsHandler *secrets, QObject *parent) : QObject(parent), secrets(secrets)
{
}

void VoipCaller::placeCall()
{
    QtConcurrent::run([=] {
#ifdef QT_DEBUG
        qDebug() << "Placing a call";
#endif

        auto sipServer = this->sipServer.toUtf8();
        auto sipUsername = this->sipUsername.toUtf8();
        auto sipPassword = this->sipPassword().toUtf8();
        auto sipTransport = this->sipTransport.toUtf8();
        auto advertisedHost = this->advertisedHost.toUtf8();
        auto localAddress = this->localAddress.toUtf8();
        auto target = this->target.toUtf8();

        CallOptions options = {
            .sipServer = sipServer.constData(),
            .sipUsername = sipUsername.constData(),
            .sipPassword = sipPassword.constData(),
            .sipServerPort = sipServerPort,
            .sipTransport = sipTransport.constData(),
            .advertisedHost = advertisedHost.constData(),
            .localAddress = localAddress.constData(),
            .localPort = localPort,
            .nat = &nat,
            .target = target.constData(),
            .timeout_ms = timeoutMs,
        };

        char *error = nullptr;
        if (PlaceCall(&options, &error) != CallResultSuccess) {
            const auto errorMessage = QString::fromUtf8(error);
            emit callFailed(errorMessage);
            qWarning() << "Error placing a call: " << errorMessage;
            std::free(error);
        } else {
            emit callSucceeded();
        }
    });
}

bool VoipCaller::removePassword()
{
    return secrets->removePassword(sipUsername, sipServer, sipServerPort, target);
}

QString VoipCaller::sipPassword()
{
    return secrets->getPassword(sipUsername, sipServer, sipServerPort, target);
}

void VoipCaller::setSipPassword(const QString &value)
{
    if (value == sipPassword()) {
        return;
    }

    secrets->setPassword(sipUsername, sipServer, sipServerPort, target, value);
    emit sipPasswordChanged();
}
