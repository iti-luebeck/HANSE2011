#ifndef SONARRETURNDATARIVAS_H
#define SONARRETURNDATARIVAS_H

#include "sonarreturndata.h"

class SonarReturnDataRivas : public SonarReturnData
{
public:
    SonarReturnDataRivas(SonarSwitchCommand& cmd, QByteArray& returnDataPacket, double headPos);
    SonarReturnDataRivas();
    SonarReturnDataRivas(const SonarReturnDataRivas& c);
//    SonarReturnDataRivas& operator =(SonarReturnDataRivas);
};

#endif // SONARRETURNDATARIVAS_H
