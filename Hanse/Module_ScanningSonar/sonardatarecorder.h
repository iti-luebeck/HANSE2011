#ifndef SONARDATARECORDER_H
#define SONARDATARECORDER_H

#include <QtCore>
#include <log4qt/logger.h>
#include "sonarreturndata.h"

class Module_ScanningSonar;

class SonarDataRecorder: public QObject
{
    Q_OBJECT
public:
    SonarDataRecorder(Module_ScanningSonar& sonar);

    virtual void start()=0;
    virtual void stop()=0;

public slots:
    void newData(const SonarReturnData data);

protected:
    Module_ScanningSonar& sonar;
    Log4Qt::Logger *logger;

    virtual void store(const SonarReturnData& data) = 0;

};

#endif // SONARDATARECORDER_H
