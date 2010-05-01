# -------------------------------------------------
# Project created by QtCreator 2010-01-09T20:44:33
# -------------------------------------------------
TARGET = HanseGUI
TEMPLATE = app
DESTDIR = ../build
LIBPATH += ../build
INCLUDEPATH += ../Framework
LIBS += -lFramework
INCLUDEPATH += ../Module_ScanningSonar
LIBS += -lModule_ScanningSonar
INCLUDEPATH += ../Module_Thruster
LIBS += -lModule_Thruster
INCLUDEPATH += ../Module_UID
LIBS += -lModule_UID
LIBS += -lModule_PressureSensor
INCLUDEPATH += ../Module_PressureSensor
LIBS += -lqextserialportd
SOURCES += main.cpp \
    mainwindow.cpp \
    modulesgraph.cpp
HEADERS += mainwindow.h \
    modulesgraph.h
FORMS += mainwindow.ui

unix:DEFINES   = _TTY_POSIX_
