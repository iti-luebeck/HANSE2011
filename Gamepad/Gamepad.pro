# -------------------------------------------------
# Project created by QtCreator 2010-03-22T16:04:51
# -------------------------------------------------
QT -= gui
QT += network
QT += test
DESTDIR = ../build
TARGET = Gamepad
CONFIG += console
CONFIG += qtestlib
CONFIG -= app_bundle
TEMPLATE = app
SOURCES += main.cpp \
    client.cpp
HEADERS += gamepad.h \
    client.h
INCLUDEPATH += Gamepaddriver
LIBPATH += Gamepaddriver
LIBS += -ldinput8
