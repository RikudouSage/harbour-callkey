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
#include <QTimer>

#include <sailfishapp.h>

#include "voipcallerfactory.h"
#include "secretshandler.h"
#include "accounts.h"
#include "themeicons.h"
#include "callkeydbus.h"

constexpr auto TRANSLATION_INSTALL_DIR = "/usr/share/harbour-callkey/translations";

namespace {

QMutex messageHandlerMutex;
QtMessageHandler defaultMessageHandler = nullptr;

QString messageTypeName(QtMsgType type)
{
    switch (type) {
    case QtDebugMsg:
        return QStringLiteral("debug");
    case QtInfoMsg:
        return QStringLiteral("info");
    case QtWarningMsg:
        return QStringLiteral("warning");
    case QtCriticalMsg:
        return QStringLiteral("critical");
    case QtFatalMsg:
        return QStringLiteral("fatal");
    }

    return QStringLiteral("unknown");
}

void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
    if (defaultMessageHandler != nullptr) {
        defaultMessageHandler(type, context, message);
    } else {
        fprintf(stderr, "%s\n", qPrintable(message));
        fflush(stderr);
    }

    if (type != QtWarningMsg) {
        return;
    }

    const QMutexLocker lock(&messageHandlerMutex);
    const auto logDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (logDir.isEmpty() || !QDir().mkpath(logDir)) {
        return;
    }

    QFile file(QDir(logDir).filePath(QStringLiteral("warnings.log")));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        return;
    }

    QTextStream stream(&file);
    stream << QDateTime::currentDateTimeUtc().toString(Qt::ISODate)
           << " [" << messageTypeName(type) << "] " << message;

    if (context.file != nullptr) {
        stream << " (" << context.file << ':' << context.line << ')';
    }

    stream << '\n';
}

}

CallKeyDBus *registerDBus(Accounts *accounts, SecretsHandler *secrets, QObject *parent);

int main(int argc, char *argv[])
{
    QScopedPointer<QGuiApplication> app(SailfishApp::application(argc, argv));
    defaultMessageHandler = qInstallMessageHandler(messageHandler);
    const bool dbusServiceMode = app->arguments().value(1) == "dbus";

    auto secrets = new SecretsHandler(app.data());
    auto accounts = new Accounts(app.data());

#ifndef HARBOUR_STORE
    if (dbusServiceMode) {
        auto callKeyDBus = registerDBus(accounts, secrets, app.data());
        if (!callKeyDBus) {
            return 1;
        }

        QObject::connect(callKeyDBus, &CallKeyDBus::actionFinished, app.data(),
            [app = app.data()](const QString &, bool success, const QString &message) {
                qWarning() << message;
                QTimer::singleShot(0, app, [app, success] {
                    app->exit(success ? 0 : 1);
                });
            });
        QTimer::singleShot(2 * 60 * 1000, app.data(), &QCoreApplication::quit);

        return app->exec();
    }
#endif

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

    auto callerFactory = new VoipCallerFactory(secrets, app.data());
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

CallKeyDBus *registerDBus(Accounts *accounts, SecretsHandler *secrets, QObject *parent) {
    auto callKeyDBus = new CallKeyDBus(accounts, secrets, parent);
    auto bus = QDBusConnection::sessionBus();

    if (!bus.registerService("cz.chrastecky.callkey")) {
        qWarning() << "Cannot register D-Bus service:" << bus.lastError().message();
        delete callKeyDBus;
        return nullptr;
    }

    if (!bus.registerObject("/cz/chrastecky/callkey", callKeyDBus, QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals)) {
        qWarning() << "Cannot register D-Bus object:" << bus.lastError().message();
        bus.unregisterService("cz.chrastecky.callkey");
        delete callKeyDBus;
        return nullptr;
    }

    return callKeyDBus;
}
