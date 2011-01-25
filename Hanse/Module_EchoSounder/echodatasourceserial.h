#ifndef ECHODATASOURCESERIAL_H
#define ECHODATASOURCESERIAL_H

#include "echodatasource.h"
#include "log4qt/logger.h"

class QextSerialPort;

class EchoDataSourceSerial : public EchoDataSource
{
public:
    EchoDataSourceSerial(Module_EchoSounder& parent);
    ~EchoDataSourceSerial();

    const EchoReturnData getNextPacket();

    bool isOpen();

    void stop();

private:
    QextSerialPort* port;

    void configurePort();

    /**
      * Logger instance for this module.
      */
    Log4Qt::Logger *logger;
};

#endif // ECHODATASOURCESERIAL_H
