#ifndef SONARDATASOURCERIVAS_H
#define SONARDATASOURCERIVAS_H

#include "sonardatasource.h"
#include "log4qt/logger.h"
#include "sonarreturndatarivas.h"

class QFile;
class QDataStream;
class Module_XsensMTi;

class SonarDataSourceRivas: public SonarDataSource
{
public:
    SonarDataSourceRivas(Module_ScanningSonar& parent, Module_XsensMTi *mti, QString sonarFile, QString mtiFile);
    ~SonarDataSourceRivas();

    const SonarReturnData getNextPacket();
    const SonarReturnDataRivas getNextPacketRivas();

    bool isOpen();

    void stop();

private:
    QFile* sonarFile;
    QFile* mtiFile;
    QTextStream* sonarStream;
    QTextStream* mtiStream;

    double lastTime;
    double currentTime;
    float lastHeading;
    float currentHeading;

    Module_XsensMTi *mti;

    /**
      * Logger instance for this module
      */
    Log4Qt::Logger *logger;

    SonarReturnDataRivas readPacket();

};

#endif // SONARDATASOURCERIVAS_H
