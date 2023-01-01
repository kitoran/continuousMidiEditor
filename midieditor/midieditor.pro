TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
DEFINES += MY_PATH=\\\"$$PWD\\\"
CONFIG += SDL
#QMAKE_CC = tcc
QMAKE_CFLAGS += -Wno-pointer-sign -Werror-implicit-function-declaration
include(../../library/stb/stb_ds.pri)
#include(../../library/gui-SDL/gui.pri)
include(../../library/gui/gui.pri)
include(../../library/extmath/extmath.pri)
SOURCES += main.c \
    melody.c \
    midi.c \
    playback.c \
    roll.c \
    save.c \
    sortediterable.c
LIBS += -lSDL2 -lSDL2_ttf -lsmf -lrtmidi

HEADERS += \
    melody.h \
    midi.h \
    midiprot.h \
    playback.h \
    roll.h \
    save.h \
    sortediterable.h

CONFIG += link_pkgconfig

PKGCONFIG += glib-2.0
