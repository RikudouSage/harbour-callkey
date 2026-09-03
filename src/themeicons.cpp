#include "themeicons.h"

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QRegularExpression>
#include <QSet>

namespace {
bool isThemeIcon(const QString &name)
{
    return name.startsWith(QStringLiteral("icon-s-"))
        || name.startsWith(QStringLiteral("icon-splus-"))
        || name.startsWith(QStringLiteral("icon-m-"))
        || name.startsWith(QStringLiteral("icon-l-"))
        || name.startsWith(QStringLiteral("icon-cover-"))
        || name.startsWith(QStringLiteral("icon-lock-"));
}

QStringList findThemeIcons()
{
    QSet<QString> icons;
    const QStringList roots = {
        QStringLiteral("/usr/share/themes"),
        QStringLiteral("/usr/share/lipstick-jolla-home-qt5"),
        QStringLiteral("/usr/share/sailfish-qml"),
    };

    for (const auto &root : roots) {
        QDirIterator iterator(root, QDir::Files, QDirIterator::Subdirectories);
        while (iterator.hasNext()) {
            iterator.next();

            const QFileInfo file(iterator.fileInfo());
            auto name = file.completeBaseName();
            if (!isThemeIcon(name)) {
                continue;
            }

            name.remove(QRegularExpression(QStringLiteral("@[0-9]+x$")));
            icons.insert(name);
        }
    }

    auto result = icons.values();
    result.sort();
    return result;
}
}

ThemeIcons::ThemeIcons(QObject *parent) : QObject(parent), m_icons(findThemeIcons())
{
}

QStringList ThemeIcons::icons() const
{
    return m_icons;
}
