TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
INCLUDEPATH += ../settings

INCLUDEPATH +=     ../../stb
SOURCES += \
    $$PWD/../settings/settings.c\
   ../../stb/stb_ds.c
HEADERS += \
    $$PWD/../settings/settings.h\
    $$PWD/../../stb/stb_ds.h
LIBS += -lexplain -lconfig
SOURCES += main.c \
#    stb_ds.c

#HEADERS += \
#    stb_ds.h
