#if(guiPers) {
include($$PWD/../settingsS/settings/settings.pri)
include($$PWD/../loadImage/loadImage.pri)
DEFINES += GUI_RESOURCE_PATH=\\\"$$PWD/gui\\\"
SOURCES +=  \
    $$PWD/gui/persistent.c \
    $$PWD/gui/draw.c
HEADERS +=  \
    $$PWD/gui/persistent.h \
    $$PWD/gui/draw.h
#}
SOURCES += \
    $$PWD/gui/gui.c \
    $$PWD/gui/stb.c \
    $$PWD/gui/gridlayout.c \
    $$PWD/gui/combobox.c \
    $$PWD/gui/guiglobals.c \


HEADERS += \
    $$PWD/gui/gui.h \
#    $$PWD/gui/stb_ds.h \
    $$PWD/gui/gridlayout.h \
    $$PWD/gui/color.h \
    $$PWD/gui/shittyintrospection.h 

INCLUDEPATH += $$PWD/gui/


CONFIG += link_pkgconfig
PKGCONFIG += x11 xrender xft
