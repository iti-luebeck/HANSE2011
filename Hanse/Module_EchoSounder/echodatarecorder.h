#ifndef ECHODATARECORDER_H
#define ECHODATARECORDER_H

#include <QtCore>
#include <log4qt/logger.h>
#include "echoreturndata.h"

class Module_EchoSounder;

class EchoDataRecorder: public QObject
{
    Q_OBJECT
public:
    EchoDataRecorder(Module_EchoSounder& sounder);

    virtual void start()=0;
    virtual void stop()=0;

public slots:
    void newData(const EchoReturnData data);

protected:
    Module_EchoSounder& sounder;
    //Log4Qt::Logger *logger;

    virtual void store(const EchoReturnData& data) = 0;
};

#endif // ECHODATARECORDER_H
