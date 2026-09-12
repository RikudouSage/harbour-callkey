#ifndef MPRISCONTROLLER_H
#define MPRISCONTROLLER_H

#include <QObject>
#include <QJsonObject>
#include <QVariantMap>
#include <QTimer>
#include <QDBusObjectPath>

class Accounts;
class AppSettings;
class VoipCaller;
class VoipCallerFactory;

class MprisController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool CanQuit READ canQuit CONSTANT)
    Q_PROPERTY(bool CanRaise READ canRaise CONSTANT)
    Q_PROPERTY(bool Fullscreen READ fullscreen WRITE setFullscreen)
    Q_PROPERTY(bool CanSetFullscreen READ canSetFullscreen CONSTANT)
    Q_PROPERTY(bool HasTrackList READ hasTrackList CONSTANT)
    Q_PROPERTY(QString Identity READ identity CONSTANT)
    Q_PROPERTY(QString DesktopEntry READ desktopEntry CONSTANT)
    Q_PROPERTY(QStringList SupportedUriSchemes READ supportedUriSchemes CONSTANT)
    Q_PROPERTY(QStringList SupportedMimeTypes READ supportedMimeTypes CONSTANT)
    Q_PROPERTY(QString PlaybackStatus READ playbackStatus CONSTANT)
    Q_PROPERTY(QString LoopStatus READ loopStatus WRITE setLoopStatus)
    Q_PROPERTY(double Rate READ rate WRITE setRate)
    Q_PROPERTY(bool Shuffle READ shuffle WRITE setShuffle)
    Q_PROPERTY(QVariantMap Metadata READ metadata NOTIFY metadataChanged)
    Q_PROPERTY(double Volume READ volume WRITE setVolume)
    Q_PROPERTY(qlonglong Position READ position CONSTANT)
    Q_PROPERTY(double MinimumRate READ minimumRate CONSTANT)
    Q_PROPERTY(double MaximumRate READ maximumRate CONSTANT)
    Q_PROPERTY(bool CanGoNext READ canGoNext CONSTANT)
    Q_PROPERTY(bool CanGoPrevious READ canGoPrevious CONSTANT)
    Q_PROPERTY(bool CanPlay READ canPlay NOTIFY canPlayChanged)
    Q_PROPERTY(bool CanPause READ canPause CONSTANT)
    Q_PROPERTY(bool CanSeek READ canSeek CONSTANT)
    Q_PROPERTY(bool CanControl READ canControl CONSTANT)

public:
    MprisController(Accounts *accounts, AppSettings *settings,
                    VoipCallerFactory *callerFactory, QObject *parent = nullptr);

    bool canQuit() const { return false; }
    bool canRaise() const { return false; }
    bool fullscreen() const { return false; }
    void setFullscreen(bool) {}
    bool canSetFullscreen() const { return false; }
    bool hasTrackList() const { return false; }
    QString identity() const;
    QString desktopEntry() const;
    QStringList supportedUriSchemes() const { return {}; }
    QStringList supportedMimeTypes() const { return {}; }
    QString playbackStatus() const;
    QString loopStatus() const { return QStringLiteral("None"); }
    void setLoopStatus(const QString &) {}
    double rate() const { return 1.0; }
    void setRate(double) {}
    bool shuffle() const { return false; }
    void setShuffle(bool) {}
    QVariantMap metadata() const;
    double volume() const { return 1.0; }
    void setVolume(double) {}
    qlonglong position() const { return 0; }
    double minimumRate() const { return 1.0; }
    double maximumRate() const { return 1.0; }
    bool canGoNext() const { return false; }
    bool canGoPrevious() const { return false; }
    bool canPlay() const;
    bool canPause() const { return false; }
    bool canSeek() const { return false; }
    bool canControl() const { return true; }

public slots:
    void Raise() {}
    void Quit() {}
    void Next() {}
    void Previous() {}
    void Pause() {}
    void PlayPause();
    void Stop() {}
    void Play();
    void Seek(qlonglong) {}
    void SetPosition(const QDBusObjectPath &, qlonglong) {}
    void OpenUri(const QString &) {}

signals:
    void metadataChanged();
    void canPlayChanged();

private slots:
    void refresh();
    void callSucceeded();
    void callFailed();
    void resetStatus();

private:
    enum class Status { Idle, Running, Success, Error };

    void configureCaller(const QJsonObject &account);
    void setStatus(Status status);
    void updateRegistration();
    void emitPropertiesChanged(const QVariantMap &properties);

    Accounts *m_accounts;
    AppSettings *m_settings;
    VoipCaller *m_caller;
    QJsonObject m_primaryAction;
    QTimer m_statusTimer;
    Status m_status = Status::Idle;
    bool m_registered = false;
};

#endif // MPRISCONTROLLER_H
