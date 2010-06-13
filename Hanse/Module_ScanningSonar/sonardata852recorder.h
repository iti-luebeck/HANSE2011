#ifndef SONARDATA852RECORDER_H
#define SONARDATA852RECORDER_H

#include "sonardatarecorder.h"
#include <QtCore>

class SonarData852Recorder : public SonarDataRecorder
{
    Q_OBJECT
public:
    SonarData852Recorder(Module_ScanningSonar& sonar);

    void start();
    void stop();

private:
    QDataStream* stream;
    QFile* file;
    void store(const SonarReturnData& data);

};

#endif // SONARDATA852RECORDER_H



