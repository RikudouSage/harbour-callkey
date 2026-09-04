#include "voipcaller.h"

#include <functional>

#include <QtConcurrent>
#include <QDebug>

#include "voip/libvoipringer.h"
#include "defer.h"

namespace {
struct CallbackData {
    bool handled = false;
    QStringList earlySuccessResponses;
    std::function<void()> callSucceeded;
};
}

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

        CallbackData callbackData = {
            .earlySuccessResponses = earlySuccessResponses,
            .callSucceeded = [this]() {
                emit callSucceeded();
            },
        };

        TargetReachedCallback onTargetReached = [](int statusCode, void *userData) {
            auto callbackData = static_cast<CallbackData *>(userData);

            if (callbackData->handled) {
                return;
            }

            bool handled = false;
            if (statusCode >= 200 && statusCode < 300 && callbackData->earlySuccessResponses.contains("2xx")) {
                handled = true;
            }
            if (statusCode == 183 && callbackData->earlySuccessResponses.contains("183")) {
                handled = true;
            }
            if (statusCode == 180 && callbackData->earlySuccessResponses.contains("180")) {
                handled = true;
            }

            if (handled) {
                callbackData->handled = true;
                callbackData->callSucceeded();
            }
        };

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
            .onTargetReached = onTargetReached,
            .onTargetReachedUserData = &callbackData,
        };

        char *error = nullptr;
        auto result = PlaceCall(&options, &error);
        if (result != CallResultSuccess) {
            defer({
                std::free(error);
            });

            if (ignoreTargetDeclineErrors && result == CallResultDeclined) {
                if (!callbackData.handled) {
                    emit callSucceeded();
                }
                return;
            }

            const auto errorMessage = QString::fromUtf8(error);
            qWarning() << "Error placing a call: " << errorMessage;
            if (!callbackData.handled) {
                emit callFailed(errorMessage);
            }
        } else {
            if (!callbackData.handled) {
                emit callSucceeded();
            }
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
