#ifndef THEMEICONS_H
#define THEMEICONS_H

#include <QObject>
#include <QStringList>

class ThemeIcons : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList icons READ icons CONSTANT)

public:
    explicit ThemeIcons(QObject *parent = nullptr);

    QStringList icons() const;

private:
    QStringList m_icons;
};

#endif // THEMEICONS_H
