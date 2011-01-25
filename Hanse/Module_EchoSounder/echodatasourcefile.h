#ifndef ECHODATASOURCEFILE_H
#define ECHODATASOURCEFILE_H

#include "echodatasource.h"
#include "log4qt/logger.h"

class QFile;
class QDataStream;

class EchoDataSourceFile : public EchoDataSource
{
public:
    EchoDataSourceFile(Module_EchoSounder& parent, QString file);
    ~EchoDataSourceFile();

    const EchoReturnData getNextPacket();

    bool isOpen();

    void stop();

private:
    QFile* file;
    QDataStream* stream;

    Log4Qt::Logger *logger;

    EchoReturnData readPacket();
};

#endif // ECHODATASOURCEFILE_H
