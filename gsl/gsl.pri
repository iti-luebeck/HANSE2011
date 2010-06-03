# TODO: fehlende dateien einchecken
win32 {
    INCLUDEPATH += $$PWD/include
    LIBPATH += $$PWD/lib
    #LIBS += -lgsl
}

# debian/ubuntu: aptitude install libgsl0-dev libblas-dev
unix:LIBS += -lgsl -lblas
