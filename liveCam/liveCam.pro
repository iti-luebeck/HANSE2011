#-------------------------------------------------
#
# Project created by QtCreator 2010-06-29T10:25:33
#
#-------------------------------------------------

TARGET = liveCam
TEMPLATE = app
DESTDIR = ../build
CONFIG += debug \
    warn_on \
    console
INCLUDEPATH += .
include(../OpenCV.pri)

SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui

