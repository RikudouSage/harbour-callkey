#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <QObject>
#include <QSettings>

class AppSettings : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool mprisEnabled READ mprisEnabled WRITE setMprisEnabled NOTIFY mprisEnabledChanged)

public:
    explicit AppSettings(QObject *parent = nullptr);

    bool mprisEnabled() const;
    void setMprisEnabled(bool enabled);

signals:
    void mprisEnabledChanged();

private:
    QSettings m_settings;
};

#endif // APPSETTINGS_H
