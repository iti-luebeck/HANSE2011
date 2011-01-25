#ifndef ECHOSWITCHCOMMAND_H
#define ECHOSWITCHCOMMAND_H

#include <QtCore>
#include <log4qt/logger.h>

class EchoSwitchCommand : public QObject
{
    Q_OBJECT
public:
    // reading from file
    EchoSwitchCommand(QByteArray fileData);

    // when building it by hand (serial)
    EchoSwitchCommand();

    EchoSwitchCommand(const EchoSwitchCommand&);

    QByteArray toSerialCmd();

    EchoSwitchCommand& operator = (EchoSwitchCommand);

    QDateTime time;
    // echosounder variables
    unsigned char range;            // Byte 03
    unsigned char startGain;        // Byte 08
    unsigned char pulseLength;      // Byte 14
    unsigned char profileMinRange;  // Byte 15 = min range in (meters/10)
    unsigned char dataPoints;       // Byte 19 / 25 = IMX header / 50 = IGX header
    unsigned char profile;          // Byte 22 0 = OFF / 1 = ON -> IPX header
    unsigned char switchDelay;      // Byte 24
    unsigned char frequency;        // Byte bla => nachgucken
    unsigned short totalBytes;
    unsigned short nToRead;

//    unsigned char stepSize;            //added cause of errors.

    QByteArray origFileHeader;

private:
        void extractHeader(const QByteArray& a);
        void clone(const EchoSwitchCommand& other);
        Log4Qt::Logger *logger;

};

#endif // ECHOSWITCHCOMMAND_H
