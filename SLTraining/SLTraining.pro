TARGET = SLTraining
include(../OpenCV.pri)

SOURCES += main.cpp \
    sonarechofilter.cpp \
    sonarswitchcommand.cpp \
    sonarreturndata.cpp \
    sonardatasourcefile.cpp \
    qgraphicsviewextended.cpp \
    SVMClassifier.cpp \
    sonarechodata.cpp \
    sltrainingui.cpp

HEADERS += \
    sonarechofilter.h \
    sonarswitchcommand.h \
    sonarreturndata.h \
    sonardatasourcefile.h \
    qgraphicsviewextended.h \
    SVMClassifier.h \
    sonarechodata.h \
    sltrainingui.h

FORMS += sltrainingui.ui
DESTDIR = ../build
CONFIG += console
