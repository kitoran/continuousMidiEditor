INCLUDEPATH += $$PWD
SOURCES *= $$PWD/loadImage.c
HEADERS += $$PWD/loadImage.h
include($$PWD/../stb/stb_image.pri)
include($$PWD/../misc/misc.pri)
include($$PWD/../stb/stb_image_write.pri)
