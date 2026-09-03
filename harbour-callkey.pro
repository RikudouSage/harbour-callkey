# NOTICE:
#
# Application name defined in TARGET has a corresponding QML filename.
# If name defined in TARGET is changed, the following needs to be done
# to match new name:
#   - corresponding QML filename must be changed
#   - desktop icon filename must be changed
#   - desktop filename must be changed
#   - icon definition filename in desktop file must be changed
#   - translation filenames have to be changed

# The name of your application
TARGET = harbour-callkey
CONFIG += sailfishapp c++20
PKGCONFIG += sailfishsecrets sailfishcrypto
QT += concurrent gui

GO_LIBDIR = /usr/share/$$TARGET/lib
INCLUDEPATH += $$PWD/voip
LIBS += -L$$PWD/voip -lvoipringer
QMAKE_RPATHDIR += $$GO_LIBDIR
libvoipringer.path = $$GO_LIBDIR
libvoipringer.files = $$PWD/voip/libvoipringer.so
INSTALLS += libvoipringer

SOURCES += src/harbour-callkey.cpp \
    src/accounts.cpp \
    src/secretshandler.cpp \
    src/voipcaller.cpp \
    src/voipcallerfactory.cpp
HEADERS += voip/libvoipringer.h \
    src/accounts.h \
    src/defer.h \
    src/secretshandler.h \
    src/voipcaller.h \
    src/voipcallerfactory.h

DISTFILES += qml/harbour-callkey.qml \
    qml/cover/CoverPage.qml \
    qml/pages/ConfigPage.qml \
    qml/pages/MainPage.qml \
    rpm/harbour-callkey.changes.in \
    rpm/harbour-callkey.changes.run.in \
    rpm/harbour-callkey.spec \
    translations/*.ts \
    harbour-callkey.desktop

SAILFISHAPP_ICONS = 86x86 108x108 128x128 172x172

# to disable building translations every time, comment out the
# following CONFIG line
CONFIG += sailfishapp_i18n sailfishapp_i18n_idbased

# German translation is enabled as an example. If you aren't
# planning to localize your app, remember to comment out the
# following TRANSLATIONS line. And also do not forget to
# modify the localized app name in the the .desktop file.
TRANSLATIONS += translations/harbour-callkey-en.ts \
                translations/harbour-callkey-cs.ts
