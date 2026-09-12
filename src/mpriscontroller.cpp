#include "mpriscontroller.h"

#include "accounts.h"
#include "appsettings.h"
#include "voipcaller.h"
#include "voipcallerfactory.h"

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusMetaType>
#include <QDBusAbstractAdaptor>

namespace {
constexpr auto serviceName = "org.mpris.MediaPlayer2.harbour_callkey";
constexpr auto objectPath = "/org/mpris/MediaPlayer2";
constexpr auto playerInterface = "org.mpris.MediaPlayer2.Player";

class MprisRootAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.mpris.MediaPlayer2")
    Q_PROPERTY(bool CanQuit READ canQuit)
    Q_PROPERTY(bool CanRaise READ canRaise)
    Q_PROPERTY(bool Fullscreen READ fullscreen WRITE setFullscreen)
    Q_PROPERTY(bool CanSetFullscreen READ canSetFullscreen)
    Q_PROPERTY(bool HasTrackList READ hasTrackList)
    Q_PROPERTY(QString Identity READ identity)
    Q_PROPERTY(QString DesktopEntry READ desktopEntry)
    Q_PROPERTY(QStringList SupportedUriSchemes READ supportedUriSchemes)
    Q_PROPERTY(QStringList SupportedMimeTypes READ supportedMimeTypes)
public:
    explicit MprisRootAdaptor(MprisController *controller) : QDBusAbstractAdaptor(controller), c(controller) {}
    bool canQuit() const { return c->canQuit(); }
    bool canRaise() const { return c->canRaise(); }
    bool fullscreen() const { return c->fullscreen(); }
    void setFullscreen(bool value) { c->setFullscreen(value); }
    bool canSetFullscreen() const { return c->canSetFullscreen(); }
    bool hasTrackList() const { return c->hasTrackList(); }
    QString identity() const { return c->identity(); }
    QString desktopEntry() const { return c->desktopEntry(); }
    QStringList supportedUriSchemes() const { return c->supportedUriSchemes(); }
    QStringList supportedMimeTypes() const { return c->supportedMimeTypes(); }
public slots:
    void Raise() { c->Raise(); }
    void Quit() { c->Quit(); }
private:
    MprisController *c;
};

class MprisPlayerAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.mpris.MediaPlayer2.Player")
    Q_PROPERTY(QString PlaybackStatus READ playbackStatus)
    Q_PROPERTY(QString LoopStatus READ loopStatus WRITE setLoopStatus)
    Q_PROPERTY(double Rate READ rate WRITE setRate)
    Q_PROPERTY(bool Shuffle READ shuffle WRITE setShuffle)
    Q_PROPERTY(QVariantMap Metadata READ metadata)
    Q_PROPERTY(double Volume READ volume WRITE setVolume)
    Q_PROPERTY(qlonglong Position READ position)
    Q_PROPERTY(double MinimumRate READ minimumRate)
    Q_PROPERTY(double MaximumRate READ maximumRate)
    Q_PROPERTY(bool CanGoNext READ canGoNext)
    Q_PROPERTY(bool CanGoPrevious READ canGoPrevious)
    Q_PROPERTY(bool CanPlay READ canPlay)
    Q_PROPERTY(bool CanPause READ canPause)
    Q_PROPERTY(bool CanSeek READ canSeek)
    Q_PROPERTY(bool CanControl READ canControl)
public:
    explicit MprisPlayerAdaptor(MprisController *controller) : QDBusAbstractAdaptor(controller), c(controller) {}
    QString playbackStatus() const { return c->playbackStatus(); }
    QString loopStatus() const { return c->loopStatus(); }
    void setLoopStatus(const QString &value) { c->setLoopStatus(value); }
    double rate() const { return c->rate(); }
    void setRate(double value) { c->setRate(value); }
    bool shuffle() const { return c->shuffle(); }
    void setShuffle(bool value) { c->setShuffle(value); }
    QVariantMap metadata() const { return c->metadata(); }
    double volume() const { return c->volume(); }
    void setVolume(double value) { c->setVolume(value); }
    qlonglong position() const { return c->position(); }
    double minimumRate() const { return c->minimumRate(); }
    double maximumRate() const { return c->maximumRate(); }
    bool canGoNext() const { return c->canGoNext(); }
    bool canGoPrevious() const { return c->canGoPrevious(); }
    bool canPlay() const { return c->canPlay(); }
    bool canPause() const { return c->canPause(); }
    bool canSeek() const { return c->canSeek(); }
    bool canControl() const { return c->canControl(); }
public slots:
    void Next() { c->Next(); }
    void Previous() { c->Previous(); }
    void Pause() { c->Pause(); }
    void PlayPause() { c->PlayPause(); }
    void Stop() { c->Stop(); }
    void Play() { c->Play(); }
    void Seek(qlonglong offset) { c->Seek(offset); }
    void SetPosition(const QDBusObjectPath &trackId, qlonglong position) { c->SetPosition(trackId, position); }
    void OpenUri(const QString &uri) { c->OpenUri(uri); }
signals:
    void Seeked(qlonglong Position);
private:
    MprisController *c;
};
}

MprisController::MprisController(Accounts *accounts, AppSettings *settings,
                                 VoipCallerFactory *callerFactory, QObject *parent)
    : QObject(parent)
    , m_accounts(accounts)
    , m_settings(settings)
    , m_caller(callerFactory->create())
{
    new MprisRootAdaptor(this);
    new MprisPlayerAdaptor(this);
    connect(accounts, &Accounts::accountsChanged, this, &MprisController::refresh);
    connect(settings, &AppSettings::mprisEnabledChanged, this, &MprisController::refresh);
    connect(m_caller, &VoipCaller::callSucceeded, this, &MprisController::callSucceeded);
    connect(m_caller, &VoipCaller::callFailed, this, &MprisController::callFailed);
    connect(&m_statusTimer, &QTimer::timeout, this, &MprisController::resetStatus);
    m_statusTimer.setSingleShot(true);
    refresh();
}

QString MprisController::identity() const { return QStringLiteral("CallKey"); }
QString MprisController::desktopEntry() const { return QStringLiteral("harbour-callkey"); }
QString MprisController::playbackStatus() const { return QStringLiteral("Paused"); }

QVariantMap MprisController::metadata() const
{
    QString title;
    switch (m_status) {
    case Status::Running: title = qtTrId("mpris.running"); break;
    case Status::Success: title = qtTrId("mpris.success"); break;
    case Status::Error: title = qtTrId("mpris.error"); break;
    case Status::Idle: title = m_primaryAction.value("name").toString(); break;
    }

    return {
        {QStringLiteral("mpris:trackid"), QVariant::fromValue(QDBusObjectPath(QStringLiteral("/org/mpris/MediaPlayer2/Track/primary")))},
        {QStringLiteral("mpris:length"), QVariant::fromValue<qlonglong>(0)},
        {QStringLiteral("xesam:title"), title},
        {QStringLiteral("xesam:artist"), QStringList{QStringLiteral("CallKey")}}
    };
}

bool MprisController::canPlay() const
{
    return m_registered && m_status != Status::Running;
}

void MprisController::PlayPause() { Play(); }

void MprisController::Play()
{
    if (!canPlay()) {
        return;
    }

    configureCaller(m_primaryAction);
    setStatus(Status::Running);
    m_caller->placeCall();
}

void MprisController::refresh()
{
    QJsonObject primary;
    const auto actionList = m_accounts->accounts();
    for (const auto &value : actionList) {
        const auto action = value.toObject();
        if (action.value("primaryAction").toBool()) {
            primary = action;
            break;
        }
    }

    if (primary != m_primaryAction) {
        m_primaryAction = primary;
        emit metadataChanged();
        emitPropertiesChanged({{QStringLiteral("Metadata"), metadata()}});
    }
    updateRegistration();
}

void MprisController::callSucceeded()
{
    if (m_status == Status::Running) {
        setStatus(Status::Success);
        m_statusTimer.start(5000);
    }
}

void MprisController::callFailed()
{
    if (m_status == Status::Running) {
        setStatus(Status::Error);
        m_statusTimer.start(15000);
    }
}

void MprisController::resetStatus() { setStatus(Status::Idle); }

void MprisController::configureCaller(const QJsonObject &action)
{
    m_caller->setProperty("sipServer", action.value("sipServer").toString());
    m_caller->setProperty("sipUsername", action.value("sipUsername").toString());
    m_caller->setProperty("sipServerPort", action.value("sipServerPort").toInt());
    m_caller->setProperty("sipTransport", action.value("sipTransport").toString(QStringLiteral("udp")));
    m_caller->setProperty("advertisedHost", action.value("advertisedHost").toString());
    m_caller->setProperty("localAddress", action.value("localAddress").toString());
    m_caller->setProperty("localPort", action.value("localPort").toInt());
    m_caller->setProperty("nat", action.value("nat").toBool(true));
    m_caller->setProperty("target", action.value("target").toString());
    m_caller->setProperty("timeoutMs", action.value("timeoutMs").toVariant());
    m_caller->setProperty("ignoreTargetDeclineErrors", action.value("ignoreTargetDeclineErrors").toBool(true));
    m_caller->setProperty("earlySuccessResponses", action.value("earlySuccessResponses").isUndefined()
                          ? QStringList{QStringLiteral("2xx"), QStringLiteral("180")}
                          : action.value("earlySuccessResponses").toVariant().toStringList());
}

void MprisController::setStatus(Status status)
{
    if (m_status == status) return;
    const bool oldCanPlay = canPlay();
    m_statusTimer.stop();
    m_status = status;
    emit metadataChanged();
    QVariantMap changed{{QStringLiteral("Metadata"), metadata()}};
    if (oldCanPlay != canPlay()) {
        emit canPlayChanged();
        changed.insert(QStringLiteral("CanPlay"), canPlay());
    }
    emitPropertiesChanged(changed);
}

void MprisController::updateRegistration()
{
    const bool eligible = m_settings->mprisEnabled() && !m_primaryAction.isEmpty();
    if (eligible == m_registered) return;

    auto bus = QDBusConnection::sessionBus();
    if (eligible) {
        if (!bus.registerService(QString::fromLatin1(serviceName))) return;
        if (!bus.registerObject(QString::fromLatin1(objectPath), this,
                                QDBusConnection::ExportAdaptors)) {
            bus.unregisterService(QString::fromLatin1(serviceName));
            return;
        }
        m_registered = true;
    } else {
        bus.unregisterObject(QString::fromLatin1(objectPath));
        bus.unregisterService(QString::fromLatin1(serviceName));
        m_registered = false;
        if (m_status != Status::Running) resetStatus();
    }
    emit canPlayChanged();
}

void MprisController::emitPropertiesChanged(const QVariantMap &properties)
{
    if (!m_registered) return;
    auto message = QDBusMessage::createSignal(QString::fromLatin1(objectPath),
                                               QStringLiteral("org.freedesktop.DBus.Properties"),
                                               QStringLiteral("PropertiesChanged"));
    message << QString::fromLatin1(playerInterface) << properties << QStringList{};
    QDBusConnection::sessionBus().send(message);
}

#include "mpriscontroller.moc"
