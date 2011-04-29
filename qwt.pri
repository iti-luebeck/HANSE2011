
win32 {
    INCLUDEPATH += $$PWD/include
    LIBPATH += $$PWD/lib/qwt
    LIBS += -lqwtd5
}

# debian/ubuntu: aptitude install libqwt5-qt4-dev
unix:LIBS += -lqwt-qt4
