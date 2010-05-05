# -------------------------------------------------
# Project created by QtCreator 2010-01-10T15:11:04
# -------------------------------------------------
TARGET = Module_ScanningSonar
TEMPLATE = lib

# CONFIG += staticlib
DEFINES += MODULE_SCANNINGSONAR_LIBRARY
SOURCES += module_scanningsonar.cpp \
    form.cpp \
    sonarreturndata.cpp \
    sonardatasource.cpp \
    sonardatasourceserial.cpp \
    sonardatasourcefile.cpp \
    sonardatarecorder.cpp \
    qsonarview.cpp
HEADERS += module_scanningsonar.h \
    Module_ScanningSonar_global.h \
    form.h \
    sonarreturndata.h \
    sonardatasource.h \
    sonardatasourceserial.h \
    sonardatasourcefile.h \
    sonardatarecorder.h \
    qsonarview.h
DESTDIR = ../build
LIBPATH += ../build
INCLUDEPATH += ../Framework
LIBS += -lFramework

# INCLUDEPATH += ../Module_SerialPort
# LIBS += -lModule_SerialPort
INCLUDEPATH += ../qextserialport
LIBS += -lqextserialportd1
FORMS += form.ui
unix:DEFINES = _TTY_POSIX_
