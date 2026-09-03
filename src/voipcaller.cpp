#include "voipcaller.h"

#include "voip/libvoipringer.h"
#include <QtConcurrent>

VoipCaller::VoipCaller(SecretsHandler *secrets, QObject *parent) : QObject(parent), secrets(secrets)
{
}

void VoipCaller::placeCall()
{
    QtConcurrent::run([=] {
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
            emit callFailed(QString::fromUtf8(error));
            std::free(error);
        } else {
            emit callSucceeded();
        }
    });
}

QString VoipCaller::sipPassword()
{
    return secrets->getPassword(sipServer, sipServerPort);
}

void VoipCaller::setSipPassword(const QString &value)
{
    if (value == sipPassword()) {
        return;
    }

    secrets->setPassword(sipServer, sipServerPort, value);
    emit sipPasswordChanged();
}
