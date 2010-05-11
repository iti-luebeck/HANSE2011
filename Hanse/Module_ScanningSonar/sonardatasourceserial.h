#ifndef SONARDATASOURCESERIAL_H
#define SONARDATASOURCESERIAL_H

#include "sonardatasource.h"
#include "log4qt/logger.h"

class QextSerialPort;

class SonarDataSourceSerial: public SonarDataSource
{
public:
    SonarDataSourceSerial(Module_ScanningSonar& parent, QString port);

    SonarReturnData* getNextPacket();

private:
    QextSerialPort* port;

    void configurePort();
    QByteArray buildSwitchDataCommand();

    /**
      * Logger instance for this module
      */
    Log4Qt::Logger *logger;

};

#endif // SONARDATASOURCESERIAL_H
