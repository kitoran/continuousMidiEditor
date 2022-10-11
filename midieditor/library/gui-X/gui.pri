#if(guiPers) {
include($$PWD/../settingsS/settings/settings.pri)
include($$PWD/../loadImage/loadImage.pri)
DEFINES += GUI_RESOURCE_PATH=\\\"$$PWD/gui\\\"
SOURCES +=  \
    $$PWD/gui/persistent.c \
    $$PWD/gui/draw.c \
    $$PWD/gui/toolbuttongroup.c
X {
SOURCES += \
    $$PWD/gui/backends/x/xbackend.c
}
HEADERS +=  \
    $$PWD/gui/persistent.h \
    $$PWD/gui/draw.h \
    $$PWD/gui/toolbuttongroup.h \
    $$PWD/gui/backends/x/backend.h
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
INCLUDEPATH += $$PWD/gui/backends/x

#LIBS += -lSDL2 #-lSDL_TTF
CONFIG += link_pkgconfig
PKGCONFIG += x11 xrender xft
