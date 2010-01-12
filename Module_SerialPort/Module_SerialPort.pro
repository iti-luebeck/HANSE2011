# -------------------------------------------------
# Project created by QtCreator 2010-01-10T18:28:53
# -------------------------------------------------
TARGET = Module_SerialPort
TEMPLATE = lib
DEFINES += MODULE_SERIALPORT_LIBRARY
SOURCES += module_serialport.cpp
HEADERS += module_serialport.h \
    Module_SerialPort_global.h
DESTDIR = ../build
LIBPATH += ../build
INCLUDEPATH += ../Framework
LIBS += -lFramework
INCLUDEPATH += ../qextserialport
LIBS += -lqextserialportd
#include(../log4qt/src/log4qt/log4qt.pri)
FORMS += module_serialport_widget.ui
