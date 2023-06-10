TEMPLATE = lib
CONFIG += console c++20
CONFIG -= app_bundle
CONFIG -= qt
INCLUDEPATH += clap/include
TARGET = retune

DESTDIR =  C:\src\exercises\build-qmake-tweakpartials-Desktop_Qt_6_4_1_MSVC2019_64bit-Debug\debug
TARGET_EXT = .clap
#install.path = C:\src\exercises\build-qmake-tweakpartials-Desktop_Qt_6_4_1_MSVC2019_64bit-Debug\debug

#install.file = retune.clap

#install.name = retune.clap
#install.extra = cp C:\src\exercises\build-qmake-tweakpartials-Desktop_Qt_6_4_1_MSVC2019_64bit-Debug\debug\retune.clap.dll C:\src\exercises\build-qmake-tweakpartials-Desktop_Qt_6_4_1_MSVC2019_64bit-Debug\debug\retune.clap
#INSTALLS += install
include(../../library/extmath/extmath.pri)
include(../../library/misc/misc.pri)
include(../../library/stb/stb_ds.pri)

CONFIG += SDL
INCLUDEPATH += C:\src\SDL2-devel-2.26.5-VC\SDL2-2.26.5\include C:\src\SDL2-devel-2.26.5-VC\SDL2_ttf-2.20.1\include
DEFINES += MY_PATH=\\\"$$PWD\\\" GUI_NO_PERSISTENT_WIDGETS
include(../../library/gui/gui.pri)
SOURCES += \
    fftRetuner.cpp \
    mainRetuner.c
#        ../additive/fft.c \
#        ../additive/fftrec.c \
#        ../additive/stft.c \
QMAKE_CFLAGS += /we4013 /we4024 /we4020 /we4047 /we4133 /O2 /fp:fast
QMAKE_CXXFLAGS += /O2 /fp:fast
HEADERS += \
    fftRetuner.h
#    ../additive/fft.h \
#    ../additive/fftrec.h \
 #    ../additive/stft.h
# LIBS += -lSDL2 -lSDL2_ttf
# LIBS += -lSDL2 -lSDL2_ttf
windows {
QMAKE_LIBDIR += C:\src\SDL2-devel-2.26.5-VC\SDL2-2.26.5\lib\x64 C:\src\SDL2-devel-2.26.5-VC\SDL2_ttf-2.20.1\lib\x64
#LIBS += /lKernel32
QMAKE_LFLAGS += user32.lib SDL2.lib SDL2_ttf.lib
DEFINES += _CRT_SECURE_NO_WARNINGS
QMAKE_CFLAGS += /Zc:preprocessor  /std:c11 /we4013 /wd4838

}
