#ifndef SONARDATA852RECORDER_H
#define SONARDATA852RECORDER_H

#include "sonardatarecorder.h"
#include <QtCore>
#include <log4qt/logger.h>

class SonarData852Recorder : public SonarDataRecorder
{
    Q_OBJECT
    LOG4QT_DECLARE_QCLASS_LOGGER
public:
    SonarData852Recorder(Module_ScanningSonar& sonar);

    void start();
    void stop();

private:
    QDataStream* stream;
    QFile* file;
    void store(const SonarReturnData& data);
    QString counter852;
};

#endif // SONARDATA852RECORDER_H



