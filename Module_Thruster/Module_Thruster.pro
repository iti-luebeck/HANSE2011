# -------------------------------------------------
# Project created by QtCreator 2010-04-30T22:03:09
# -------------------------------------------------
TARGET = Module_Thruster
TEMPLATE = lib
DEFINES += MODULE_THRUSTER_LIBRARY
SOURCES += module_thruster.cpp \
    thruster_form.cpp
HEADERS += module_thruster.h \
    Module_Thruster_global.h \
    thruster_form.h
DESTDIR = ../build
LIBPATH += ../build
INCLUDEPATH += ../Framework
LIBS += -lFramework
INCLUDEPATH += ../Module_UID
LIBS += -lModule_UID
FORMS += thruster_form.ui
