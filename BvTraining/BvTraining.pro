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
    blobtraining.cpp
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
    blobtraining.h
FORMS += trainingwindow.ui
DESTDIR = ../build
INCLUDEPATH += "..\OpenCV\include\opencv" \
    "..\OpenCV\include\blobs"
LIBPATH += "..\OpenCV\lib"
LIBPATH += ../build
LIBS += -lcv200 \
    -lcxcore200 \
    -lcvaux200 \
    -lml200 \
    -lcxts200 \
    -lhighgui200
