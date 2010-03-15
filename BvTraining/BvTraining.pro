# -------------------------------------------------
# Project created by QtCreator 2010-03-15T11:33:17
# -------------------------------------------------
TARGET = BvTraining
TEMPLATE = app
SOURCES += main.cpp \
    trainingwindow.cpp \
    imageprocessor.cpp \
    SVMClassifier.cpp
HEADERS += trainingwindow.h \
    imageprocessor.h \
    SVMClassifier.h
FORMS += trainingwindow.ui
DESTDIR = ../build
INCLUDEPATH += "..\OpenCV\include\opencv"
LIBPATH += "..\OpenCV\lib"
LIBPATH += ../build
LIBS += -lcv200 \
    -lcxcore200 \
    -lcvaux200 \
    -lml200 \
    -lcxts200 \
    -lhighgui200
