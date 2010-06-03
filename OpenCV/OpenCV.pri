
# untested!
win32 {
    INCLUDEPATH += $$PWD\include\opencv
    LIBPATH += lib/DShow/lib
    LIBS += "..\VisualSLAM\lib\libvideoInput.a"
    LIBS += -lcv -lcxcore -lcvaux -lhighgui
    LIBS += -lddraw \
        -ldxguid \
        -lstrmbase \
        -lstrmiids \
        -luuid \
        -lole32 \
        -loleaut32 \
    LIBS += "..\lib\opencv\libopencv_features2d211.dll.a"
}

# debian/ubuntu: aptitude install libcv-dev libcvaux-dev libhighgui-dev libswscale-dev
unix:LIBS += -lcv -lcxcore -lcvaux #-lhighgui -lswscale
