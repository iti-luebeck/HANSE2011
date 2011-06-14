#include "echodata852recorder.h"
#include "echoreturndata.h"
#include "echodatarecorder.h"
#include <QtCore>
#include "module_echosounder.h"
#include <Framework/dataloghelper.h>

EchoData852Recorder::EchoData852Recorder(Module_EchoSounder& s)
    : EchoDataRecorder(s)
{
    file = NULL;
    stream = NULL;
}

void EchoData852Recorder::start()
{
    if (file != NULL)
        stop();
    counter852 = sounder.getSettingsValue("counter852").toString();
    sounder.setSettingsValue("counter852", sounder.getSettingsValue("counter852").toInt()+1);
    counter852 = "echolog"+counter852+".852";
    file = new QFile(DataLogHelper::getLogDir()+counter852);
    //file = new QFile(DataLogHelper::getLogDir()+"echolog.852");
    if (file->open(QFile::WriteOnly | QFile::Truncate | QFile::Append)) {
        stream = new QDataStream(file);
        stream->setVersion(QDataStream::Qt_4_6);
    } else {
        logger()->error("Could not open file "+file->fileName());
    }
}

void EchoData852Recorder::stop()
{
    if (file != NULL) {
        file->close();
        delete file;
        file = NULL;
    }

    if (stream != NULL) {
        delete stream;
        stream = NULL;
    }
}

void EchoData852Recorder::store(const EchoReturnData &edata)
{
    if (!edata.isPacketValid()) {
        logger()->warn("I'm not writing invalid packets to file, sorry.");
        return;
    }

    // File Header
    *stream << (unsigned char)0x38; // ASCII '8'
    *stream << (unsigned char)0x35; // ASCII '5'
    *stream << (unsigned char)0x32; // ASCII '2'

    // nToReadIndex, Total Bytes and nToRead
    if (edata.getDataBytes()==0) {
        *stream << (unsigned char)0;    // IPX data
        *stream << (unsigned short)128; // Data bytes
        *stream << (unsigned short)13;  // Number of Bytes from the echosounder
    } else if (edata.getDataBytes()==252) {
        *stream << (unsigned char)2;    // IMX data
        *stream << (unsigned short)384; // Data bytes
        *stream << (unsigned short)265; // Number of Bytes from the echosounder
    } else if (edata.getDataBytes()==500) {
        *stream << (unsigned char)3;    // IGX data
        *stream << (unsigned short)640; // Data bytes
        *stream << (unsigned short)513; // Number of Bytes from the echosounder
    } else {
        logger()->error("Bad number of data bytes. cannot write to file: "+QString::number(edata.getDataBytes()));
        return;
    }

    QDateTime time = edata.switchCommand.time;
    QLocale l = QLocale(QLocale::English, QLocale::UnitedKingdom);
    stream->writeRawData(l.toString(time,"dd-MMM-yyyy").toUpper().toAscii(), 12); // Date, 12 Bytes
    stream->writeRawData(l.toString(time,"HH:mm:ss").toAscii(), 9);     // Time, 9 Bytes
    stream->writeRawData(l.toString(time,".z").left(3).toAscii(), 4);   // Hundredth of Seconds, 4 Bytes

    *stream << (unsigned char)0;    // Reserved, always 0
    *stream << (unsigned char)0;    // Reserved, always 0
    *stream << (unsigned char)0;    // Reserved, always 0
    *stream << (unsigned char)0;    // Reserved, always 0

    unsigned char flags = 0; // Dir, Xdcr, Mode, Step
//    if (edata.isCWDirection())
//        flags |= 0x80;
//    flags |= 0x08;
//    if (edata.switchcommand.stepSize==2)
//      flags |= 0x01;

    /*if (edata.switchCommand.origFileHeader.length()<38){
       logger()->error("data.switchCommand.origFileHeader has length "+QString::number(edata.switchCommand.origFileHeader.length()));
    } else {
       flags = edata.switchCommand.origFileHeader[37];
    }*/
    *stream <<  flags;

    *stream << edata.switchCommand.startGain; // Start Gain
    *stream << (unsigned char)0;    // Sector Size, Reserved, always 0
    *stream << (unsigned char)0;    // Train Angle, Reserved, always 0
    *stream << (unsigned char)0;    // Reserved, always 0
    *stream << (unsigned char)20;   // Reserved, always 20
    *stream << (unsigned char)9;    // Reserved, always 9
    *stream << edata.switchCommand.pulseLength; // Pulse Length
    // bislang wurden 44 Bytes abgearbeitet.


    //*stream << (unsigned char)0;    // Profile, 0=Off
    stream->writeRawData(QByteArray(42,0), 42); // Bytes 45-86, Zero Fill
    *stream << edata.switchCommand.frequency; // Operating Frequency

    *stream << (unsigned char)0x11; // Head ID, 0x11 Echosounder
    stream->writeRawData(QByteArray(11,0), 11); // Bytes 89-99, Zero Fill

    stream->writeRawData(edata.packet, edata.packet.length());

    if (edata.getDataBytes()==0) {
        stream->writeRawData(QByteArray(15,0), 15); // Bytes 113-127, Zero Fill
    } else if (edata.getDataBytes()==252) {
        stream->writeRawData(QByteArray(19,0), 19); // Bytes 365-383, Zero Fill
    } else if (edata.getDataBytes()==500) {
        stream->writeRawData(QByteArray(27,0), 27); // Bytes 613-639, Zero Fill
    }

}

