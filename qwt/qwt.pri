
win32 {
    INCLUDEPATH += $$PWD/include
    LIBPATH += $$PWD/lib
    LIBS += -lqwtd5
}

# debian/ubuntu: aptitude install libcv-dev libcvaux-dev libhighgui-dev libswscale-dev
unix:LIBS += -lqwt-qt4
