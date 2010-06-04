
win32 {
    INCLUDEPATH += $$PWD/include
    LIBPATH += $$PWD/lib/opencv
    LIBS += -lcv210 -lcxcore210 -lcvaux210 -lhighgui210

    # DShow
    # LIBS += -ldxguid -lstrmbase -lstrmiids -luuid -lole32 -loleaut32
}

# debian/ubuntu: aptitude install libcv-dev libcvaux-dev libhighgui-dev libswscale-dev
unix:LIBS += -lcv -lcxcore -lcvaux #-lhighgui -lswscale
