
win32 {
    INCLUDEPATH += $$PWD/include
    LIBPATH += $$PWD/lib
    LIBS += -lgsl -lgslcblas
}

# debian/ubuntu: aptitude install libgsl0-dev libblas-dev
unix:LIBS += -lgsl -lblas
