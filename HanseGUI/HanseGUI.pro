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
INCLUDEPATH += ../Module_PressureSensor
LIBS += -lModule_PressureSensor
INCLUDEPATH += ../Module_ThrusterControlLoop
LIBS += -lModule_ThrusterControlLoop
INCLUDEPATH += ../Module_HandControl
LIBS += -lModule_HandControl
LIBS += -lqextserialportd1
SOURCES += main.cpp \
    mainwindow.cpp \
    modulesgraph.cpp \
    healthmodel.cpp \
    datamodel.cpp
HEADERS += mainwindow.h \
    modulesgraph.h \
    healthmodel.h \
    datamodel.h
FORMS += mainwindow.ui
unix:DEFINES = _TTY_POSIX_
