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
    Q_ASSERT(clone.length()==getDataBytes());
    return clone;
}

int SonarReturnData::THCDecoder(char byteLO, char byteHI)
{
    int high =   (byteHI & 0x7E) >> 1;
    int low = ( ((byteHI & 0x01) << 7) | (byteLO & 0x7F) );

    return high << 8 | low;
}

int SonarReturnData::getProfilerRange()
{
    return THCDecoder(packet[8], packet[9]);
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
    Q_ASSERT(packet[0] == 'I');
    Q_ASSERT(packet[2] == 'X');

    if (packet.length()==13) {
        Q_ASSERT(packet[1] == 'P');
        Q_ASSERT(getDataBytes() == 0);
    }
    if (packet.length()==265) {
        Q_ASSERT(packet[1] == 'M');
        Q_ASSERT(getDataBytes() == 265);
    }
    if (packet.length()==513) {
        Q_ASSERT(packet[1] == 'G');
        Q_ASSERT(getDataBytes() == 513);
    }

    // todo: check serial status

    // todo: maybe, actually, return false???

    // always the same
    Q_ASSERT((char)packet[3] == 0x11);

    return true;
}
