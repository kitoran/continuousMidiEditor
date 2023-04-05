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
    actions.c \
    editorinstance.cpp \
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
windows {
    DEFINES += _CRT_SECURE_NO_WARNINGS
    INCLUDEPATH += C:\src\SDL2-devel-2.26.1-VC\SDL2-2.26.1\include C:\src\SDL2-devel-2.26.1-VC\SDL2_ttf-2.20.1\include
    CONFIG -= console
    LIBS += /lKernel32
    QMAKE_CXXFLAGS += /Zc:preprocessor /wd4838
#    -lSDL2main -lShell32
    QMAKE_LFLAGS += user32.lib
    QMAKE_CFLAGS += /std:c11 /we4013 /wd4838
    SDL2.dll.path = C:\src\SDL2-devel-2.26.1-VC\SDL2-2.26.1\
        INSTALLS += SDL2.dll
    #    QMAKE_CXXFLAGS += /D__cplusplus #wtf why was this not defined in the first place
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
