#include "sonarreturndata.h"

SonarReturnData::SonarReturnData(QByteArray &returnDataPacket)
{
    this->packet = returnDataPacket;
}

QByteArray SonarReturnData::getEchoData()
{
    QByteArray clone = QByteArray(packet);
    clone.remove(0,12); // remove header
    clone.remove(getDataBytes(), 1); // remove termination byte
    return clone;
}

int SonarReturnData::THCDecoder(char byteLO, char byteHI)
{
    int high =   (byteHI & 0x7E) >> 1;
    int low = ( ((byteHI & 0x01) << 7) | (byteLO & 0x7F) );

    return high << 8 | low;
}

int SonarReturnData::THCHeadPosDecoder(char byteLO, char byteHI)
{
    int high =   (byteHI & 0x3E) >> 1;
    int low = ( ((byteHI & 0x01) << 7) | (byteLO & 0x7F) );

    return high << 8 | low;
}

bool SonarReturnData::isSwitchesAccepted()
{
    return (packet[4] & 0x40) >> 6;
}

bool SonarReturnData::isCharacterOverrun()
{
    return (packet[4] & 0x80) >> 7;
}

bool SonarReturnData::isCWDirection()
{
    return (packet[6] & 0x40) >> 6;
}

double SonarReturnData::getHeadPosition()
{
    int headPos = THCHeadPosDecoder(packet[5], packet[6]);
    return 0.15*(headPos - 1400);
}

int SonarReturnData::getDataBytes()
{
    return THCDecoder(packet[10], packet[11]);
}

int SonarReturnData::getRange()
{
    return packet[7];
}

bool SonarReturnData::isPacketValid()
{
    // ID Bytes
    Q_ASSERT(packet[0] == 'I');
    Q_ASSERT(packet[2] == 'X');

    if (packet.length()==265) {
        Q_ASSERT(packet[1] == 'M');
        Q_ASSERT(getDataBytes() == 252);
    }
    if (packet.length()==513) {
        Q_ASSERT(packet[1] == 'G');
        Q_ASSERT(getDataBytes() == 500);
    }

    Q_ASSERT(isSwitchesAccepted());
    Q_ASSERT(!isCharacterOverrun());

    // todo: maybe, actually, return false???

    // HeadID
    Q_ASSERT((char)packet[3] == 0x10);

    // Termination byte
    Q_ASSERT(packet[packet.length()-1] == 0xFC);

    return true;
}
