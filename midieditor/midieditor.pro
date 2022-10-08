TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
DEFINES += MY_PATH=\\\"$$PWD\\\"
DEFINES += SDL

include(../../library/stb/stb_ds.pri)
#include(../../library/gui-SDL/gui.pri)

SOURCES += main.c
LIBS += -lSDL2 -lSDL2_ttf
