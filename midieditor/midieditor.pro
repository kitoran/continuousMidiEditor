TEMPLATE = app
CONFIG -= app_bundle
CONFIG -= qt
DEFINES += MY_PATH=\\\"$$PWD\\\"
#MAX_STRING_LEN=100
CONFIG += SDL
#QMAKE_CC = tcc
linux {
    CONFIG += console
    QMAKE_CFLAGS += -Wno-pointer-sign -Werror-implicit-function-declaration
}
windows {
    DEFINES += _CRT_SECURE_NO_WARNINGS
    INCLUDEPATH += C:\src\SDL2-devel-2.26.1-VC\SDL2-2.26.1\include C:\src\SDL2-devel-2.26.1-VC\SDL2_ttf-2.20.1\include
    CONFIG -= console
    LIBS += -lSDL2main -lShell32
    SDL2.dll.path = C:\src\SDL2-devel-2.26.1-VC\SDL2-2.26.1\
    INSTALLS += SDL2.dll
}
include(../../library/stb/stb_ds.pri)
#include(../../library/gui-SDL/gui.pri)
include(../../library/gui/gui.pri)
include(../../library/extmath/extmath.pri)
SOURCES += main.c \
    mainprogram.c \
    melody.c \
    midi.c \
    playback.c \
    roll.c \
    save.c \
    sortediterable.c
LIBS += -lSDL2 -lSDL2_ttf
linux {
   LIBS += -lsmf -lrtmidi
}
QMAKE_LIBDIR += C:\src\SDL2-devel-2.26.1-VC\SDL2-2.26.1\lib\x64 C:\src\SDL2-devel-2.26.1-VC\SDL2_ttf-2.20.1\lib\x64
HEADERS += \
    mainprogram.h \
    melody.h \
    midi.h \
    midiprot.h \
    playback.h \
    roll.h \
    save.h \
    sortediterable.h

#CONFIG += link_pkgconfig

#PKGCONFIG += glib-2.0
