TEMPLATE = subdirs

SUBDIRS += \
    application \
    gui
application.depends = gui
