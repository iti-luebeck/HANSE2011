# -------------------------------------------------
# Project created by QtCreator 2010-05-01T13:53:58
# -------------------------------------------------
TARGET = Module_ThrusterControlLoop
TEMPLATE = lib
DEFINES += MODULE_THRUSTERCONTROLLOOP_LIBRARY
SOURCES += module_thrustercontrolloop.cpp \
    tcl_form.cpp
HEADERS += module_thrustercontrolloop.h \
    Module_ThrusterControlLoop_global.h \
    tcl_form.h
DESTDIR = ../build
LIBPATH += ../build
INCLUDEPATH += ../Framework
LIBS += -lFramework
INCLUDEPATH += ../Module_Thruster
LIBS += -lModule_Thruster
INCLUDEPATH += ../Module_PressureSensor
LIBS += -lModule_PressureSensor
FORMS += tcl_form.ui
