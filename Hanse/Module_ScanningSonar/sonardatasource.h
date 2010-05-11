#ifndef SONARDATASOURCE_H
#define SONARDATASOURCE_H

#include "sonarreturndata.h"

class SonarReturnData;
class Module_ScanningSonar;

class SonarDataSource
{
public:
    SonarDataSource(Module_ScanningSonar& parent);

    virtual SonarReturnData* getNextPacket() = 0;

protected:
    Module_ScanningSonar& parent;

    // TODO: stop
};

#endif // SONARDATASOURCE_H
