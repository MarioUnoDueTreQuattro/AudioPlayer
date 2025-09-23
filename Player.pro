QT       += core gui multimedia widgets
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

TARGET = AudioPlayer
TEMPLATE = app
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    clickableslider.cpp \
    main.cpp \
    settings.cpp \
    singleinstance.cpp \
    widget.cpp

HEADERS += \
    clickableslider.h \
    settings.h \
    singleinstance.h \
    widget.h

FORMS += \
    settings.ui \
    widget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc

RC_FILE = appicon.rc

win32:RC_ICONS = icons8-play-96.ico

DISTFILES += \
    appicon.rc

VERSION = 1.0.0.2
DEFINES += APP_VERSION=\\\"$$VERSION\\\"
