TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += link_pkgconfig
LIBS += -lexplain
#-lXft
PKGCONFIG += x11 xrender xft
INCLUDEPATH += ../gui
#LIBS += -L../../bui
SOURCES += main.c

unix:!macx: LIBS += -L$$OUT_PWD/../gui/ -lgui

INCLUDEPATH += $$PWD/../gui
INCLUDEPATH += $$PWD/../../settingsS/settings
INCLUDEPATH += /usr/include/freetype2
DEPENDPATH += $$PWD/../gui

unix:!macx: PRE_TARGETDEPS += $$OUT_PWD/../gui/libgui.a
include(../../stb/stb_image.pri)
include(../../loadImage/loadImage.pri)
DEFINES += MY_PATH=$$PWD
