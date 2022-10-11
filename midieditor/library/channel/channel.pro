#-------------------------------------------------
#
# Project created by QtCreator 2021-03-26T13:38:44
#
#-------------------------------------------------

QT       -= core gui

TARGET = channel
TEMPLATE = lib

#DEFINES += CHANNEL_LIBRARY

SOURCES +=

HEADERS += \
        channel.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
