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

INCLUDEPATH += "..\include" \
    "..\include\opencv" \

LIBS += "..\OpenCV\lib\libcv210.dll.a" \
    "..\OpenCV\lib\libcxcore210.dll.a" \
    "..\OpenCV\lib\libcvaux210.dll.a" \
    "..\OpenCV\lib\libhighgui210.dll.a"
