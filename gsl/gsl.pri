win32 {
    INCLUDEPATH += $$PWD\include
    LIBS += $$PWD\lib\libgsl.dll.a
}

# debian/ubuntu: aptitude install libgsl0-dev libblas-dev
unix:LIBS += -lgsl -lblas
