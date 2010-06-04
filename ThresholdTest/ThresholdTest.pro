#-------------------------------------------------
#
# Project created by QtCreator 2010-06-04T10:55:20
#
#-------------------------------------------------

TARGET = ThresholdTest
TEMPLATE = app
DESTDIR = ../build


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui

INCLUDEPATH += "..\include"

LIBS += "..\lib\opencv\libcv210.dll.a" \
    "..\lib\opencv\libcxcore210.dll.a" \
    "..\lib\opencv\libcvaux210.dll.a" \
    "..\lib\opencv\libhighgui210.dll.a"
