# -------------------------------------------------
# Project created by QtCreator 2010-01-21T13:44:32
# -------------------------------------------------
TARGET = Module_UID
TEMPLATE = lib
DEFINES += MODULE_UID_LIBRARY
DESTDIR = ../build
LIBPATH += ../build

# use framework
INCLUDEPATH += ../Framework
LIBS += -lFramework

# use qextserialport
INCLUDEPATH += ../qextserialport
LIBS += -lqextserialportd
SOURCES += module_uid.cpp \
    form.cpp
HEADERS += module_uid.h \
    Module_UID_global.h \
    form.h
FORMS += form.ui

unix:DEFINES   = _TTY_POSIX_

