#ifndef SONARDATASOURCE_H
#define SONARDATASOURCE_H

#include "sonarreturndata.h"

class SonarReturnData;
class Module_ScanningSonar;

class SonarDataSource
{
public:
    SonarDataSource(Module_ScanningSonar& parent);
    ~SonarDataSource();

    virtual const SonarReturnData getNextPacket() = 0;

    virtual bool isOpen() = 0;

    virtual void stop() = 0;

    double sonarTime;

protected:
    Module_ScanningSonar& parent;

    // TODO: stop
};

#endif // SONARDATASOURCE_H
