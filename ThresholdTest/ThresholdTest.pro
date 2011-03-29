#-------------------------------------------------
#
# Project created by QtCreator 2010-06-04T10:55:20
#
#-------------------------------------------------

TARGET = ThresholdTest
TEMPLATE = app
DESTDIR = ../build

include(../OpenCV.pri)

SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui

INCLUDEPATH += "..\include"
