# -------------------------------------------------
# Project created by QtCreator 2010-05-02T10:14:59
# -------------------------------------------------
TARGET = Module_HandControl
TEMPLATE = lib
DEFINES += MODULE_HANDCONTROL_LIBRARY
SOURCES += module_handcontrol.cpp \
    handcontrol_form.cpp \
    server.cpp
HEADERS += module_handcontrol.h \
    Module_HandControl_global.h \
    handcontrol_form.h \
    server.h
DESTDIR = ../build
LIBPATH += ../build
INCLUDEPATH += ../Framework
LIBS += -lFramework
INCLUDEPATH += ../Module_Thruster
LIBS += -lModule_Thruster
INCLUDEPATH += ../Module_ThrusterControlLoop
LIBS += -lModule_ThrusterControlLoop
FORMS += handcontrol_form.ui
QT += network
