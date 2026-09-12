#include "appsettings.h"

#include <QStandardPaths>

namespace {
constexpr auto mprisEnabledKey = "mprisEnabled";
}

AppSettings::AppSettings(QObject *parent)
    : QObject(parent)
    , m_settings(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/settings.ini",
                 QSettings::IniFormat)
{
}

bool AppSettings::mprisEnabled() const
{
    return m_settings.value(mprisEnabledKey, false).toBool();
}

void AppSettings::setMprisEnabled(bool enabled)
{
    if (mprisEnabled() == enabled) {
        return;
    }

    m_settings.setValue(mprisEnabledKey, enabled);
    m_settings.sync();
    emit mprisEnabledChanged();
}
