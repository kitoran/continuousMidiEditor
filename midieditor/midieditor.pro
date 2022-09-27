TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
include(../../library/stb/stb_ds.pri)

SOURCES += main.c
LIBS += -lSDL2 -lSDL2_ttf
