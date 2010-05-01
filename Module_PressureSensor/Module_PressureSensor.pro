# -------------------------------------------------
# Project created by QtCreator 2010-05-01T08:59:41
# -------------------------------------------------
TARGET = Module_PressureSensor
TEMPLATE = lib
DEFINES += MODULE_PRESSURESENSOR_LIBRARY
SOURCES += module_pressuresensor.cpp \
    pressure_form.cpp
HEADERS += module_pressuresensor.h \
    Module_PressureSensor_global.h \
    pressure_form.h
DESTDIR = ../build
LIBPATH += ../build
INCLUDEPATH += ../Framework
LIBS += -lFramework
INCLUDEPATH += ../Module_UID
LIBS += -lModule_UID
FORMS += pressure_form.ui
