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
    SonarDataRecorder(Module_ScanningSonar& sonar, bool formatCSV);

    void start();
    void reset();
    void stop();

public slots:
    void newData(const SonarReturnData data);

private:
    Module_ScanningSonar& sonar;
    QTextStream* stream;
    QFile* file;
    Log4Qt::Logger *logger;
    QTime time;
    bool formatCSV;

    void storeCSV(const SonarReturnData& data);
    void store852(const SonarReturnData& data);

};

#endif // SONARDATARECORDER_H
