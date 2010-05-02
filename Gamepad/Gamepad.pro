# -------------------------------------------------
# Project created by QtCreator 2010-03-22T16:04:51
# -------------------------------------------------
QT -= gui
QT += network
QT += test
TARGET = Gamepad
CONFIG += console
CONFIG += qtestlib
CONFIG -= app_bundle
TEMPLATE = app
SOURCES += main.cpp \
    client.cpp
HEADERS += gamepad.h \
    client.h
INCLUDEPATH += "C:\Programme\Microsoft DirectX SDK (February 2010)\Include"
LIBS += -lwinmm \
    -L"C:\Programme\Microsoft DirectX SDK (February 2010)\Lib\x86" \
    -ldinput8 \
    -dxguid
