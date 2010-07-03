# -------------------------------------------------
# Project created by QtCreator 2010-03-15T11:33:17
# -------------------------------------------------
TARGET = BvTraining
TEMPLATE = app
include(../OpenCV.pri)
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
    surfclassifier.cpp \
    feature.cpp \
    surf/utils.cpp \
    surf/surf.cpp \
    surf/ipoint.cpp \
    surf/integral.cpp \
    surf/fasthessian.cpp
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
    surfclassifier.h \
    feature.h \
    surf/utils.h \
    surf/surflib.h \
    surf/surf.h \
    surf/responselayer.h \
    surf/kmeans.h \
    surf/ipoint.h \
    surf/integral.h \
    surf/fasthessian.h
FORMS += trainingwindow.ui \
    ../Hanse/Module_VisualSLAM/form_visualslam.ui \
    ../Hanse/Module_Navigation/waypointdialog.ui
DESTDIR = ../build
CONFIG += console
