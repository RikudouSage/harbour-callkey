#ifndef VOIPCALLERFACTORY_H
#define VOIPCALLERFACTORY_H

#include <QObject>

#include "secretshandler.h"
#include "voipcaller.h"

class VoipCallerFactory : public QObject
{
    Q_OBJECT
public:
    explicit VoipCallerFactory(SecretsHandler *secrets, QObject *parent = nullptr);

    Q_INVOKABLE VoipCaller* create();

private:
    SecretsHandler *secrets;

};

#endif // VOIPCALLERFACTORY_H
