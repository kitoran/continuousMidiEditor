TEMPLATE = lib
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += c++20
DEFINES += MY_PATH=\\\"$$PWD/\\\" GUI_NO_PERSISTENT_WIDGETS REAPER
#MAX_STRING_LEN=100
CONFIG += SDL
TARGET = reaper_midieditor
DESTDIR = C:\Users\my\AppData\Roaming\REAPER\UserPlugins
#QMAKE_CC = tcc
linux {
    CONFIG += console
    QMAKE_CFLAGS += -Wno-pointer-sign -Werror-implicit-function-declaration
}
windows {
    DEFINES += _CRT_SECURE_NO_WARNINGS
    INCLUDEPATH += C:\src\SDL2-devel-2.26.5-VC\SDL2-2.26.5\include C:\src\SDL2-devel-2.26.5-VC\SDL2_ttf-2.20.1\include
    CONFIG -= console
    LIBS += /lwinmm
    #Kernel32
    QMAKE_CXXFLAGS += /Zc:preprocessor /wd4838 /RTC1 /MDd
#    -lSDL2main -lShell32
    QMAKE_LFLAGS += user32.lib  SDL2_ttf.lib SDL2.lib freetype.lib gdi32.lib Shell32.lib Setupapi.lib Advapi32.lib \
                Ole32.lib Version.lib Imm32.lib  	OleAut32.lib
    QMAKE_CFLAGS += /std:c11 /we4013 /wd4838 /we4020  /we4133 /RTC1 /MDd
    SDL2.lib.path = C:\src\SDL2-2.28.2\build\Release
    #C:\src\SDL2-devel-2.26.5-VC\SDL2-2.26.5\
    INSTALLS += SDL2.lib
    RC_FILE += resourses.rc
#        RC_ICONS += resources/magnetic-horizontal.png
    #    QMAKE_CXXFLAGS += /D__cplusplus #wtf why was this not defined in the first place
}
include(../library/stb/stb_ds.pri)
#include(../../library/gui-SDL/gui.pri)
include(../library/gui/gui.pri)
include(../library/extmath/extmath.pri)
SOURCES += \
    actionsReaper.cpp \
    editorinstance.cpp \
    mainprogram.c \
    melody.c \
    midi.c \
    playback.c \
    pluginEntryPoint.cpp \
    roll.c \
    save.c \
    sortediterable.c
#main.c \
#LIBS += -lSDL2 -lSDL2_ttf
linux {
   LIBS += -lsmf -lrtmidi
}
QMAKE_LIBDIR += C:\src\SDL2-2.28.2\build\Release C:\src\SDL2_ttf-2.20.2\build\Release C:\src\staticlibs
#C:\src\SDL2-devel-2.26.5-VC\SDL2-2.26.5\lib\x64 C:\src\SDL2-devel-2.26.5-VC\SDL2_ttf-2.20.1\lib\x64
HEADERS += \
    12edo.h \
    actions.h \
    actionsReaper.h \
    editorinstance.h \
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

FORMS += \
    settingsform.ui
OTHER_FILES += README.md

DISTFILES += \
    C:/src/library/gui/gui/icons/gui_resources.rc \
    resourses.rc
