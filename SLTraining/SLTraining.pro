TARGET = SLTraining
include(../OpenCV.pri)

SOURCES += main.cpp \
    mainwindow.cpp \
    sonarechofilter.cpp \
    sonarswitchcommand.cpp \
    sonarreturndata.cpp \
    sonardatasourcefile.cpp \
    qgraphicsviewextended.cpp \
    SVMClassifier.cpp
HEADERS += \
    sonarechofilter.h \
    sonarswitchcommand.h \
    sonarreturndata.h \
    sonardatasourcefile.h \
    qgraphicsviewextended.h \
    SVMClassifier.h \
    mainwindow.h

FORMS += mainwindow.ui
DESTDIR = ../build
CONFIG += console