#ifndef SONARDATASOURCEFILE_H
#define SONARDATASOURCEFILE_H

#include "sonarreturndata.h"

class QFile;
class QDataStream;

class SonarDataSourceFile: public QObject
{ Q_OBJECT
public:
    SonarDataSourceFile(QObject *parent, QString file);
    ~SonarDataSourceFile();

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
