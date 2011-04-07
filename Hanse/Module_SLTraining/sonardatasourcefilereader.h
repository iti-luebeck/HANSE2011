#ifndef SONARDATASOURCEFILEREADER_H
#define SONARDATASOURCEFILEREADER_H

#include "Module_ScanningSonar/sonarreturndata.h"

class QFile;
class QDataStream;

class SonarDataSourceFileReader: public QObject
{ Q_OBJECT
public:
    SonarDataSourceFileReader(QObject *parent, QString file);
    ~SonarDataSourceFileReader();

    const SonarReturnData getNextPacket();

    bool isOpen();

    void stop();

   // parameter
    QDateTime startTime;
    int fileReaderDelay;


private:
    QFile* file;
    QDataStream* stream;

    SonarReturnData readPacket();

};

#endif // SONARDATASOURCEFILE_H
