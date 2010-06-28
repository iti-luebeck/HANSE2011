
win32 {
    INCLUDEPATH += $$PWD/include
    LIBPATH += $$PWD/lib/opencv
    LIBPATH += $$PWD/lib/videoInput
    LIBS += -lcv210 -lcxcore210 -lcvaux210 -lhighgui210 -lml210

    # DShow
    LIBS += -lvideoInput -lddraw -ldxguid -lstrmbase -lstrmiids -luuid -lole32 -loleaut32
}

# debian/ubuntu: aptitude install libcv-dev libcvaux-dev libhighgui-dev libswscale-dev
unix:LIBS += -lcv -lcxcore -lcvaux #-lhighgui -lswscale
