TARGET = SLTraining
include(../OpenCV.pri)

SOURCES += main.cpp \
    mainwindow.cpp \
    svmclassifier.cpp \
    sonarechofilter.cpp \
    sonarswitchcommand.cpp \
    sonarreturndata.cpp \
    sonardatasourcefile.cpp \
    qgraphicsviewextended.cpp
HEADERS += mainwindow.h \
    svmclassifier.h \
    sonarechofilter.h \
    sonarswitchcommand.h \
    sonarreturndata.h \
    sonardatasourcefile.h \
    qgraphicsviewextended.h

FORMS += mainwindow.ui
DESTDIR = ../build
CONFIG += console
