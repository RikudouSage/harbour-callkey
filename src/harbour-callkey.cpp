#ifdef QT_QML_DEBUG
#include <QtQuick>
#endif

#include <QQuickView>
#include <QScopedPointer>
#include <QGuiApplication>
#include <QtQml>
#include <QQmlEngine>
#include <QTranslator>
#include <QDBusConnection>
#include <QDBusError>

#include <sailfishapp.h>

#include "voipcallerfactory.h"
#include "secretshandler.h"
#include "accounts.h"
#include "themeicons.h"
#include "callkeydbus.h"

constexpr auto TRANSLATION_INSTALL_DIR = "/usr/share/harbour-callkey/translations";

void registerDBus(Accounts *accounts, SecretsHandler *secrets, QObject *parent);

int main(int argc, char *argv[])
{
    QScopedPointer<QGuiApplication> app(SailfishApp::application(argc, argv));
    QScopedPointer<QQuickView> v(SailfishApp::createView());

    QTranslator *defaultLang = new QTranslator(app.data());
    if (!defaultLang->load("harbour-callkey-en", TRANSLATION_INSTALL_DIR)) {
        qWarning() << "Could not load English translation file!";
    }
    QCoreApplication::installTranslator(defaultLang);

    QTranslator *translator = new QTranslator(app.data());
    if (!translator->load(QLocale(QLocale::system().name()), "harbour-callkey", "-", TRANSLATION_INSTALL_DIR)) {
        qWarning() << "Could not load translations for" << QLocale::system().name();
    }
    QCoreApplication::installTranslator(translator);

#ifdef QT_DEBUG
    v->rootContext()->setContextProperty("isDebug", true);
#else
    v->rootContext()->setContextProperty("isDebug", false);
#endif

    auto secrets = new SecretsHandler(app.data());
    auto callerFactory = new VoipCallerFactory(secrets, app.data());
    auto accounts = new Accounts(app.data());
    auto themeIcons = new ThemeIcons(app.data());

    qmlRegisterUncreatableType<VoipCaller>("cz.chrastecky", 1, 0, "VoipCaller", "VoipCaller instances are created by VoipCallerFactory");

    v->rootContext()->setContextProperty("callerFactory", callerFactory);
    v->rootContext()->setContextProperty("accounts", accounts);
    v->rootContext()->setContextProperty("themeIcons", themeIcons);

    registerDBus(accounts, secrets, app.data());

    v->setSource(SailfishApp::pathToMainQml());
    v->show();

    return app->exec();
}

void registerDBus(Accounts *accounts, SecretsHandler *secrets, QObject *parent) {
    auto callKeyDBus = new CallKeyDBus(accounts, secrets, parent);
    auto bus = QDBusConnection::sessionBus();

    if (!bus.registerService("dev.rikudou.callkey")) {
        callKeyDBus->deleteLater();
        qWarning() << "Cannot register D-Bus service:" << bus.lastError().message();
        return;
    }

    if (!bus.registerObject("/dev/rikudou/callkey", callKeyDBus, QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals)) {
        callKeyDBus->deleteLater();
        qWarning() << "Cannot register D-Bus object:" << bus.lastError().message();
        bus.unregisterService("dev.rikudou.callkey");
        return;
    }
}
