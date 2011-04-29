
# debian/ubuntu: apt-get install libboost-thread-dev
unix:LIBS += -lboost_thread
unix:DEFINES += ENABLE_XSENS

# common for both OS
HEADERS += $$PWD/module_xsensmti.h \
    $$PWD/xsens_form.h

SOURCES += $$PWD/xsens_form.cpp $$PWD/module_xsensmti.cpp

FORMS += $$PWD/xsens_form.ui

unix:HEADERS += $$PWD/MTi/MTMessage.h \
    $$PWD/MTi/MTi.h \
    $$PWD/MTi/MTDataTypes.h \
    $$PWD/MTi/CerealPort.h

unix:SOURCES +=  $$PWD/MTi/MTMessage.cpp \
    $$PWD/MTi/MTi.cpp \
    $$PWD/MTi/CerealPort.cpp
