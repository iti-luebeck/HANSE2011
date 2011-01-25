#ifndef ECHORETURNDATA_H
#define ECHORETURNDATA_H

#include <QtCore>
#include <QtGui>
#include "echoswitchcommand.h"

class EchoReturnData : public QObject
{

public:
    enum HeaderType
    {
        /**
          * 252 Data points in this package
          */
        IMX,

        /**
          * 500 Data points in this package
          */
        IGX,

        /**
          * Zarro Data points in this package
          */
        IPX
    };

public:
    EchoReturnData(EchoSwitchCommand& cmd, QByteArray& returnDataPacket);

    EchoReturnData();
    EchoReturnData(const EchoReturnData& c);

    /**
      * Check if this packet is valid.
      * All specs are ok and no error bits are set.
      */
    bool isPacketValid() const;

    // ???
    bool isSwitchesAccepted() const;
    bool isCharacterOverrun() const;

    /**
      * Returns current sensor range in meters.
      */
    int getRange() const;

    /**
      * Number of Data points in this packet.
      */
    int getDataBytes() const;

    /**
      * The actual echo data.
      * format unknown?
      */
    QByteArray getEchoData() const;

    /**
      * The unmodified data packet as received from the sounder.
      */
    QByteArray packet;

    EchoReturnData& operator = (EchoReturnData);

    EchoSwitchCommand switchCommand;

private:

    /**
      * Decodes a 14 bit integer from two consequtive packet bytes.
      *
      * The guys at Imagenex must have smoked some really nifty
      * shit when they devised this encoding.
      *
      * (see page 6/7 of the echo sounder protocol spec for
      * details.)
      */
    int THCDecoder(char byteLO, char byteHI) const;

    // Nanana, rausnehmen?
    /**
      * Decodes the Head Position from two packet bytes.
      * very similar to THCDecoder(), only difference is that this
      * method masks another bit.
      */
    int THCHeadPosDecoder(char byteLO, char byteHI) const;

};

#endif // ECHORETURNDATA_H
