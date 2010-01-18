#ifndef SONARRETURNDATA_H
#define SONARRETURNDATA_H

#include <QObject>
#include <QGraphicsItem>

class SonarReturnData : public QObject
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

    SonarReturnData(QByteArray& returnDataPacket);

    /**
      * Checks if this packet is valid, i.e. if the packet fulfills the spec
      * and none of the error bits in the status field are set.
      */
    bool isPacketValid();

    /**
      * XXX: Some kind of error code?
      */
    bool isSwitchesAccepted();

    /**
      * XXX: Another error code?
      */
    bool isCharacterOverrun();

    /**
      * Returns the sensor range in meters.
      */
    int getRange();

    double getHeadPosition();

    /**
      * Number of Data points in this packet.
      */
    int getDataBytes();

    /**
      * Returns true if the sonar head is currently moving in a clockwise direction
      */
    bool isCWDirection();

    /**
      * The actual echo data. XXX: actual format??
      */
    QByteArray getEchoData();

private:

    /**
      * The unmodified data packet as received from the sonar
      */
    QByteArray packet;

    /**
      * Decodes a 14 bit integer from two consequtive packet bytes.
      *
      * The guys at Imagenex must have smoked some really nifty
      * shit when they devised this encoding.
      *
      * (see page 6/7 of the echo sounder protocol spec for
      * details.)
      */
    int THCDecoder(char byteLO, char byteHI);

    /**
      * Decodes the Head Position from two packet bytes.
      * very similar to THCDecoder(), only difference is that this
      * method masks another bit.
      */
    int THCHeadPosDecoder(char byteLO, char byteHI);

};

#endif // SONARRETURNDATA_H
