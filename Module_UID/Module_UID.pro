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
    form_uid.cpp \
    QtUID.cpp
HEADERS += module_uid.h \
    Module_UID_global.h \
    form_uid.h \
    QtUID.h
FORMS += form_uid.ui
unix:DEFINES = _TTY_POSIX_
