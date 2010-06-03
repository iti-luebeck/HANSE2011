
win32 {
    INCLUDEPATH += $$PWD/include
    LIBPATH += $$PWD/lib
    LIBS += -lqwt5
}

# debian/ubuntu: aptitude install libcv-dev libcvaux-dev libhighgui-dev libswscale-dev
unix:LIBS += -lqwt-qt4
