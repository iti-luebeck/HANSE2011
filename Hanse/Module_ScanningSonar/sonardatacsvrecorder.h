#ifndef SONARDATACSVRECORDER_H
#define SONARDATACSVRECORDER_H

#include "sonardatarecorder.h"
#include <QtCore>

class SonarDataCSVRecorder : public SonarDataRecorder
{
    Q_OBJECT
public:
    SonarDataCSVRecorder(Module_ScanningSonar& sonar);

    void start();
    void stop();

private:
    QTextStream* stream;
    QFile* file;
    void store(const SonarReturnData& data);

};

#endif // SONARDATACSVRECORDER_H
