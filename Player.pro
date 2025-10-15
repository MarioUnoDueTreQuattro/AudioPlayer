QT       += core gui multimedia widgets
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

TARGET = AudioPlayer
TEMPLATE = app
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
#    3rdparty/qhotkey/QHotkey/qhotkey.cpp \
#    3rdparty/qhotkey/QHotkey/qhotkey_win.cpp \
    audiofader.cpp \
    audiotag.cpp \
    clickablelabel.cpp \
    clickableslider.cpp \
    escawarelineedit.cpp \
    infowidget.cpp \
    main.cpp \
    playlistdurationdelegate.cpp \
    playlistsortmodel.cpp \
    playlisttable.cpp \
    settings.cpp \
    settingsmanager.cpp \
    singleinstance.cpp \
    systemvolumecontroller.cpp \
    tagloaderworker.cpp \
    widget.cpp

HEADERS += \
#    3rdparty/qhotkey/QHotkey/qhotkey.h \
#    3rdparty/qhotkey/QHotkey/qhotkey_p.h \
    audiofader.h \
    audiotag.h \
    clickablelabel.h \
    clickableslider.h \
    escawarelineedit.h \
    infowidget.h \
    playlistdurationdelegate.h \
    playlistsortmodel.h \
    playlisttable.h \
    settings.h \
    settingsmanager.h \
    singleinstance.h \
    systemvolumecontroller.h \
    tagloaderworker.h \
    utility.h \
    widget.h

FORMS += \
    infowidget.ui \
    playlisttable.ui \
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
    appicon.rc \
    icons8-play-96.ico

LIBS += -lole32
#win32:LIBS += -lPropsys

# TagLib include path
#INCLUDEPATH += C:/Qt/taglib-2.1.1
INCLUDEPATH += C:/Qt/taglib/include/taglib
#INCLUDEPATH += C:/Qt/taglib/taglib/toolkit
#INCLUDEPATH += C:/Qt/taglib-2.1.1/taglib
#INCLUDEPATH += C:/Qt/taglib-2.1.1/taglib/toolkit
# TagLib library path and linking
# TagLib static library
#LIBS += "C:/Qt/taglib-2.1.1/build-taglib-2.1.1-Desktop_Qt_5_15_2_MinGW_32bit-Debug/taglib/libtag.a"
#LIBS += -LC:/Qt/taglib-2.1.1/build-taglib-2.1.1-Desktop_Qt_5_15_2_MinGW_32bit-Debug/taglib/ -ltaglib
# Link against the import library for the DLL
#LIBS += "C:/Qt/taglib-2.1.1/build-taglib-2.1.1-Desktop_Qt_5_15_2_MinGW_32bit-Debug/taglib/libtag.dll.a"
LIBS += "C:/Qt/taglib/lib/libtag.dll.a"
DEFINES += TAGLIB_DLL
#HEADERS += \
#    $$PWD/3rdparty/qhotkey/QHotkey/qhotkey.h \
#    $$PWD/3rdparty/qhotkey/QHotkey/qhotkey_p.h

#SOURCES += \
#    $$PWD/3rdparty/qhotkey/QHotkey/qhotkey.cpp \
#    $$PWD/3rdparty/qhotkey/QHotkey/qhotkey_win.cpp
    VERSION = 1.0.0.3
DEFINES += APP_VERSION=\\\"$$VERSION\\\"
DEFINES += QT_MESSAGELOGCONTEXT
