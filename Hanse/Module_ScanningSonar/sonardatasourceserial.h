#ifndef SONARDATASOURCESERIAL_H
#define SONARDATASOURCESERIAL_H

#include "sonardatasource.h"
#include "log4qt/logger.h"

class QextSerialPort;

class SonarDataSourceSerial: public SonarDataSource
{
public:
    SonarDataSourceSerial(Module_ScanningSonar& parent);

    const SonarReturnData getNextPacket();

    bool isOpen();

private:
    QextSerialPort* port;

    void configurePort();

    /**
      * Logger instance for this module
      */
    Log4Qt::Logger *logger;

};

#endif // SONARDATASOURCESERIAL_H
