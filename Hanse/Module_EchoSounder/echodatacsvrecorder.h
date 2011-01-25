#ifndef ECHODATACSVRECORDER_H
#define ECHODATACSVRECORDER_H

#include "echodatarecorder.h"
#include <QtCore>

class EchoDataCSVRecorder : public EchoDataRecorder
{
    Q_OBJECT
    LOG4QT_DECLARE_QCLASS_LOGGER

public:
    EchoDataCSVRecorder(Module_EchoSounder& echo);

    void start();
    void stop();

private:
    QTextStream* stream;
    QFile* file;
    void store(const EchoReturnData &data);
};

#endif // ECHODATACSVRECORDER_H
