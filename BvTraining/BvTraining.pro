# -------------------------------------------------
# Project created by QtCreator 2010-03-15T11:33:17
# -------------------------------------------------
TARGET = BvTraining
TEMPLATE = app
SOURCES += main.cpp \
    trainingwindow.cpp \
    imageprocessor.cpp \
    SVMClassifier.cpp \
    Blobs/ComponentLabeling.cpp \
    Blobs/BlobResult.cpp \
    Blobs/BlobOperators.cpp \
    Blobs/BlobContour.cpp \
    Blobs/blob.cpp \
    blobtraining.cpp \
    surftraining.cpp \
    helpers.cpp \
    surfclassifier.cpp
HEADERS += trainingwindow.h \
    imageprocessor.h \
    SVMClassifier.h \
    Blobs/ComponentLabeling.h \
    Blobs/BlobResult.h \
    Blobs/BlobProperties.h \
    Blobs/BlobOperators.h \
    Blobs/BlobLibraryConfiguration.h \
    Blobs/BlobContour.h \
    Blobs/blob.h \
    blobtraining.h \
    surftraining.h \
    helpers.h \
    surftraining.h \
    helpers.h \
    surfclassifier.h
FORMS += trainingwindow.ui \
    ../Hanse/Module_VisualSLAM/form_visualslam.ui \
    ../Hanse/Module_Navigation/waypointdialog.ui
DESTDIR = ../build
INCLUDEPATH += "../include/opencv"
LIBPATH += "../OpenCV/lib"
LIBPATH += ../build
LIBS += "..\lib\opencv\libcv210.dll.a" \
    "..\lib\opencv\libcxcore210.dll.a" \
    "..\lib\opencv\libcvaux210.dll.a" \
    "..\lib\opencv\libhighgui210.dll.a" \
    "..\lib\opencv\libml210.dll.a"
CONFIG += console
