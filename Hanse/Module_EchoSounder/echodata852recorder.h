#ifndef ECHODATA852RECORDER_H
#define ECHODATA852RECORDER_H

#include "echodatarecorder.h"
#include <QtCore>
#include <log4qt/logger.h>

class EchoData852Recorder: public EchoDataRecorder
{
    Q_OBJECT
    LOG4QT_DECLARE_QCLASS_LOGGER
public:
    EchoData852Recorder(Module_EchoSounder& echo);

    void start();
    void stop();

private:
    QDataStream* stream;
    QFile* file;
    void store(const EchoReturnData& data);
    QString counter852;
};

#endif // ECHODATA852RECORDER_H
