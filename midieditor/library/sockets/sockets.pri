HEADERS += $$PWD/server.h
SOURCES += $$PWD/server.c
INCLUDEPATH += $$PWD
include ($$PWD/../eventLoop/eventLoop.pri)
