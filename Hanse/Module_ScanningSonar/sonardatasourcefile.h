#ifndef SONARDATASOURCEFILE_H
#define SONARDATASOURCEFILE_H

#include "sonardatasource.h"
#include "log4qt/logger.h"

class QFile;
class QDataStream;

class SonarDataSourceFile: public SonarDataSource
{
public:
    SonarDataSourceFile(Module_ScanningSonar& parent, QString file);

    SonarReturnData* getNextPacket();

private:
    QFile* file;
    QDataStream* stream;

    /**
      * Logger instance for this module
      */
    Log4Qt::Logger *logger;

};

#endif // SONARDATASOURCEFILE_H
