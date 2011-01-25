#include "echoreturndata.h"
#include "QtCore"

EchoReturnData::EchoReturnData()
{
}

EchoReturnData::EchoReturnData(const EchoReturnData &c) : QObject(){
    this->packet = c.packet;
    this->switchCommand = c.switchCommand;
}

EchoReturnData::EchoReturnData(EchoSwitchCommand &cmd, QByteArray &returnDataPacket){
    this->packet = returnDataPacket;
    this->switchCommand = cmd;
}

EchoReturnData& EchoReturnData::operator =(EchoReturnData other){
    this->packet = other.packet;
    this->switchCommand = other.switchCommand;
    return *this;
}

QByteArray EchoReturnData::getEchoData() const{
    QByteArray clone = packet;
    clone.remove(0,12);                 // removal of header.
    clone.remove(getDataBytes(), 1);    // removal of termination byte.
    qDebug() << QString::number(clone.size());
    return clone;
}

int EchoReturnData::THCDecoder(char byteLO, char byteHI) const{
    int high =   (byteHI & 0x7E) >> 1;
    int low = ( ((byteHI & 0x01) << 7) | (byteLO & 0x7F) );

    return high << 8 | low;
}

bool EchoReturnData::isSwitchesAccepted() const
{
    return (packet[4] & 0x40) >> 6;
}

bool EchoReturnData::isCharacterOverrun() const
{
    return (packet[4] & 0x80) >> 7;
}

int EchoReturnData::getDataBytes() const
{
    return THCDecoder(packet[10], packet[11]);
}

int EchoReturnData::getRange() const
{
    return packet[7];
}

bool EchoReturnData::isPacketValid() const
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
    if ((char)packet[3] != 0x11)
        return false;

    // Termination byte
    if (packet[packet.length()-1] != (char)0xFC)
        return false;

    return true;
}
