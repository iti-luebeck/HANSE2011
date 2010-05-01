# -------------------------------------------------
# Project created by QtCreator 2010-01-09T20:51:15
# -------------------------------------------------
QT -= gui
TARGET = Framework
TEMPLATE = lib

# CONFIG += staticlib
DEFINES += FRAMEWORK_LIBRARY
DESTDIR = ../build
include(log4qt/log4qt.pri)
SOURCES += robotmodule.cpp \
    healthstatus.cpp
HEADERS += framework.h \
    Framework_global.h \
    robotmodule.h \
    healthstatus.h
