#include "voipcallerfactory.h"

VoipCallerFactory::VoipCallerFactory(SecretsHandler *secrets, QObject *parent) : QObject(parent), secrets(secrets)
{
}

VoipCaller *VoipCallerFactory::create()
{
    return new VoipCaller(secrets, this);
}
