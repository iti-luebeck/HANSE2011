#include "sonarreturndatarivas.h"

SonarReturnDataRivas::SonarReturnDataRivas(SonarSwitchCommand& cmd, QByteArray& returnDataPacket, double headPos)
{
    this->packet = returnDataPacket;
    this->switchCommand = cmd;
    this->headPos = headPos;
    this->valid = true;
    this->echo = packet;
    this->range = 50;
}

SonarReturnDataRivas::SonarReturnDataRivas()
{
}

SonarReturnDataRivas::SonarReturnDataRivas(const SonarReturnDataRivas& c) : ::SonarReturnData(c)
{
}

//SonarReturnDataRivas& SonarReturnDataRivas::operator =(SonarReturnDataRivas other) {
//    this->packet = other.packet;
//    this->switchCommand = other.switchCommand;
//    this->headPos = other.headPos;
//    return *this;
//}
