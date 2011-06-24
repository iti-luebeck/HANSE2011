#include "echoswitchcommand.h"


EchoSwitchCommand::EchoSwitchCommand(){
    logger = Log4Qt::Logger::logger("EchoSwitchCommand");
    time = QDateTime::currentDateTime();
}

EchoSwitchCommand::EchoSwitchCommand(QByteArray fileData){
    logger = Log4Qt::Logger::logger("EchoSwitchCommand");
    extractHeader(fileData);
}

EchoSwitchCommand::EchoSwitchCommand(const EchoSwitchCommand& other) : QObject() {
    logger = Log4Qt::Logger::logger("EchoSwitchCommand");
    clone(other);
}

QByteArray EchoSwitchCommand::toSerialCmd(){
    QByteArray a;
    a.resize(27);
    a[0] = 0xFE;            // Switch data header
    a[1] = 0x44;            // Switch data header
    a[2] = 0x11;            // Head ID
    a[3] = range;           // Range
    a[4] = 0;               // Reserved
    a[5] = 0;               // Reserved
    a[6] = 0;               // Reserved
    a[7] = 0;               // Reserved
    a[8] = startGain;       // Start Gain
    a[9] = 0;               // Reserved
    a[10] = 20;             // Absorption
    a[11] = 0;              // Reserved
    a[12] = 0;              // Reserved
    a[13] = 0;              // Reserved
    a[14] = pulseLength;    // PulseLength
    a[15] = profileMinRange;// Profile Minimum Range: Fuer uns eig immer 0 oder nicht?
    a[16] = 0;              // Reserved
    a[17] = 0;              // Reserved
    a[18] = 0;              // Reserved
    a[19] = dataPoints;     // DataPoints: 25 or 50
    a[20] = 0;              // Reserved
    a[21] = 0;              // Reserved
    a[22] = profile;        // Profile: 0 or 1
    a[23] = 0;              // Reserved
    a[24] = switchDelay;    // Switch delay
    a[25] = 0;              // Frequency: for echo sounder always 0
    a[26] = 0xFD;           // Termination Byte

    return a;
}

EchoSwitchCommand& EchoSwitchCommand::operator =(EchoSwitchCommand other){
    clone(other);
    return *this;
}

void EchoSwitchCommand::extractHeader(const QByteArray &a)
{
    if(a.length()<100){
        logger->error("852 header not long enough: "+QString::number(a.length()));
    }

    QDataStream stream(a);
    stream.skipRawData(4); // skip 0-3

    // genaue Funktion?
    stream >> totalBytes >> nToRead;

    // read date and time.
    char date[12];
    stream.readRawData(date, 12);
    char time[9];
    stream.readRawData(time, 9);
    char hs[4];
    stream.readRawData(hs, 4);

    QString fullString(date);
    fullString.append(" ");
    fullString.append(time);
    fullString.append(hs);

    this->time = QLocale(QLocale::English, QLocale::UnitedKingdom).toDateTime(fullString, "dd-MMM-yyyy HH:mm:ss:z");

    stream.skipRawData(4); // skip 33-36
    stream.skipRawData(1); // skip 37 => Unterschied zwischen IGX und IMX?

    stream >> startGain;

    stream.skipRawData(5); // skip 39-43

    stream >> pulseLength;

    stream >> profile; // test der verschiedenen Profile

    stream.skipRawData(41); //skip 46-86

    stream >> frequency;

    unsigned char headID;
    stream >> headID;

    if(headID != 0x11){
        logger->error("read bad value as head ID. correct file?");
    }

    // logger->trace("totalBytes=" + QString::number(totalBytes));
    // logger->trace("nToRead=" + QString::number(nToRead));
    // logger->trace("startGain=" + QString::number(startGain));
    // logger->trace("DateTime=" + this->time.toString("ddd MMM d yyyy HH:mm:ss.zzz"));

    origFileHeader = a;
}

void EchoSwitchCommand::clone(const EchoSwitchCommand &other){
    this->time = other.time;
    this->range = other.range;
    this->startGain = other.startGain;
    this->pulseLength = other.pulseLength;
    this->profileMinRange = other.profileMinRange;
    this->dataPoints = other.dataPoints;
    this->profile = other.profile;
    this->frequency = other.frequency;
    this->switchDelay = other.switchDelay;
    this->totalBytes = other.totalBytes;
    this->nToRead = other.nToRead;
    this->origFileHeader = other.origFileHeader;
}
