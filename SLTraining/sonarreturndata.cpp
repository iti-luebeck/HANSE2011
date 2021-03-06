#include "sonarreturndata.h"
#include "QtCore"

SonarReturnData::SonarReturnData()
{
}

SonarReturnData::SonarReturnData(const SonarReturnData& c)
    : QObject()
{
    this->packet = c.packet;
    this->switchCommand = c.switchCommand;
}

SonarReturnData::SonarReturnData(SonarSwitchCommand& cmd, QByteArray& returnDataPacket)
{
    this->packet = returnDataPacket;
    this->switchCommand = cmd;
}

SonarReturnData& SonarReturnData::operator =(SonarReturnData other) {
    this->packet = other.packet;
    this->switchCommand = other.switchCommand;
    return *this;
}

QByteArray SonarReturnData::getEchoData() const
{
    QByteArray clone = packet;
    clone.remove(0,12); // remove header
    clone.remove(getDataBytes(), 1); // remove termination byte
    return clone;
}

int SonarReturnData::THCDecoder(char byteLO, char byteHI) const
{
    int high =   (byteHI & 0x7E) >> 1;
    int low = ( ((byteHI & 0x01) << 7) | (byteLO & 0x7F) );

    return high << 8 | low;
}

int SonarReturnData::THCHeadPosDecoder(char byteLO, char byteHI) const
{
    int high =   (byteHI & 0x3E) >> 1;
    int low = ( ((byteHI & 0x01) << 7) | (byteLO & 0x7F) );

    return high << 8 | low;
}

bool SonarReturnData::isSwitchesAccepted() const
{
    return (packet[4] & 0x40) >> 6;
}

bool SonarReturnData::isCharacterOverrun() const
{
    return (packet[4] & 0x80) >> 7;
}

bool SonarReturnData::isCWDirection() const
{
    return (packet[6] & 0x40) >> 6;
}

double SonarReturnData::getHeadPosition() const
{
    int headPos = THCHeadPosDecoder(packet[5], packet[6]);
    return 0.15*(headPos - 1400);
}

int SonarReturnData::getDataBytes() const
{
    return THCDecoder(packet[10], packet[11]);
}

int SonarReturnData::getRange() const
{
    return packet[7];
}

bool SonarReturnData::isPacketValid() const
{
    if (packet.size()<8)
        return false;

    // ID Bytes
    if (packet[0] != 'I' || packet[2] != 'X')
        return false;

    if (packet.length()==265) {
        if (packet[1] != 'M')
            return false;
        if (getDataBytes() != 252)
            return false;
    }
    if (packet.length()==513) {
        if (packet[1] != 'G')
            return false;
        if (getDataBytes() != 500)
            return false;
    }

    if (!isSwitchesAccepted())
        return false;
    if (isCharacterOverrun())
        return false;

    // HeadID
    if ((char)packet[3] != 0x10)
        return false;

    // Termination byte
    if (packet[packet.length()-1] != (char)0xFC)
        return false;

    return true;
}
