# -------------------------------------------------
# Project created by QtCreator 2010-01-09T20:51:15
# -------------------------------------------------
QT -= gui
TARGET = Framework
TEMPLATE = lib
DEFINES += FRAMEWORK_LIBRARY
DESTDIR = ../build
#include(../log4qt/src/log4qt/log4qt.pri)
SOURCES += robotmodule.cpp
HEADERS += framework.h \
    Framework_global.h \
    robotmodule.h
