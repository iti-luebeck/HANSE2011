#ifndef ECHODATASOURCE_H
#define ECHODATASOURCE_H

#include "echoreturndata.h"

class EchoReturnData;
class Module_EchoSounder;

class EchoDataSource
{
public:
    EchoDataSource(Module_EchoSounder& parent);
    ~EchoDataSource();

    virtual const EchoReturnData getNextPacket() = 0;

    virtual bool isOpen() = 0;

    virtual void stop() = 0;

protected:
    Module_EchoSounder& parent;

    // TODO: stop
};

#endif // ECHODATASOURCE_H
