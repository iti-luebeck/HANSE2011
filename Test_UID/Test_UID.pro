#-------------------------------------------------
#
# Project created by QtCreator 2010-01-21T14:34:52
#
#-------------------------------------------------

QT       -= gui

TARGET = Test_UID
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += main.cpp

DESTDIR = ../build
LIBPATH += ../build
INCLUDEPATH += ../Framework
LIBS += -lFramework
INCLUDEPATH += ../Module_UID
LIBS += -lModule_UID
LIBS += -lqextserialportd
unix:DEFINES   = _TTY_POSIX_
