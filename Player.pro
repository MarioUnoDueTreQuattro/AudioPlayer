QT       += core gui multimedia widgets
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets sql

CONFIG += c++11

TARGET = AudioPlayer
TEMPLATE = app
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
INCLUDEPATH += widgets

SOURCES += \
#    3rdparty/qhotkey/QHotkey/qhotkey.cpp \
#    3rdparty/qhotkey/QHotkey/qhotkey_win.cpp \
    about.cpp \
    audio_fader.cpp \
    audio_tag.cpp \
    database_manager.cpp \
    info_widget.cpp \
    main.cpp \
    playlist_delegates.cpp \
    playlist_helper.cpp \
    playlist_sortmodel.cpp \
    playlist_table.cpp \
    settings.cpp \
    settings_manager.cpp \
    single_instance.cpp \
    system_volume_controller.cpp \
    tag_loader_worker.cpp \
    utility.cpp \
    widget.cpp \
    widgets/clickable_label.cpp \
    widgets/clickable_slider.cpp \
    widgets/elided_header_view.cpp \
    widgets/esc_aware_lineedit.cpp \
    widgets/fading_slider.cpp \
    widgets/hover_menu_toolbutton.cpp

HEADERS += \
#    3rdparty/qhotkey/QHotkey/qhotkey.h \
#    3rdparty/qhotkey/QHotkey/qhotkey_p.h \
    about.h \
    audio_fader.h \
    audio_tag.h \
    database_manager.h \
    info_widget.h \
    playlist_delegates.h \
    playlist_helper.h \
    playlist_sortmodel.h \
    playlist_table.h \
    settings.h \
    settings_manager.h \
    single_instance.h \
    system_volume_controller.h \
    tag_loader_worker.h \
    utility.h \
    widget.h \
    widgets/clickable_label.h \
    widgets/clickable_slider.h \
    widgets/elided_header_view.h \
    widgets/esc_aware_lineedit.h \
    widgets/fading_slider.h \
    widgets/hover_menu_toolbutton.h

FORMS += \
    about.ui \
    info_widget.ui \
    playlist_table.ui \
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
    VERSION = 1.0.0.5
DEFINES += APP_VERSION=\\\"$$VERSION\\\"
DEFINES += QT_MESSAGELOGCONTEXT
# Imposta il livello di ottimizzazione a 0 (nessuna ottimizzazione) per il Debug
CONFIG(debug, debug|release) {
    QMAKE_CXXFLAGS += -O0
}
