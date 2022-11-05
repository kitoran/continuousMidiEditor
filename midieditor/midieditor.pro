TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
DEFINES += MY_PATH=\\\"$$PWD\\\"
DEFINES += SDL
QMAKE_CFLAGS += -Wno-pointer-sign -Werror-implicit-function-declaration
include(../../library/stb/stb_ds.pri)
#include(../../library/gui-SDL/gui.pri)
include(library/gui-X/gui.pri)
include(../../library/extmath/extmath.pri)

SOURCES += main.c \
    melody.c \
    roll.c \
    sortediterable.c
LIBS += -lSDL2 -lSDL2_ttf

HEADERS += \
    melody.h \
    roll.h \
    sortediterable.h
