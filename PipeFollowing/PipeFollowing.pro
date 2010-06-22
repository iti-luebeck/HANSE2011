# -------------------------------------------------
# Project created by QtCreator 2010-05-18T11:58:24
# -------------------------------------------------
TARGET = PipeFollowing
TEMPLATE = app
SOURCES += main.cpp \
    mainwindow.cpp \
    PipeFollow.cpp
HEADERS += mainwindow.h \
    PipeFollow.h
FORMS += mainwindow.ui
DESTDIR = ../build
INCLUDEPATH += "../include/opencv"
LIBPATH += "../lib"
LIBPATH += ../build
LIBS += -lcv210 \
    -lcxcore210 \
    -lcvaux210 \
    -lml210 \
    -lcxts210 \
    -lhighgui210
CONFIG += console
