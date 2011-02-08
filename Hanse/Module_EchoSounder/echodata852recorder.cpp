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

    file = new QFile(DataLogHelper::getLogDir()+"echolog.852");
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

    *stream << (unsigned char)0x38;
    *stream << (unsigned char)0x35;
    *stream << (unsigned char)0x32;
    if (edata.getDataBytes()==0) {
        *stream << (unsigned char)0;
        *stream << (unsigned short)128;
        *stream << (unsigned short)13;
    } else if (edata.getDataBytes()==252) {
        *stream << (unsigned char)2;
        *stream << (unsigned short)384;
        *stream << (unsigned short)265;
    } else if (edata.getDataBytes()==500) {
        *stream << (unsigned char)3;
        *stream << (unsigned short)640;
        *stream << (unsigned short)513;
    } else {
        logger()->error("Bad number of data bytes. cannot write to file: "+QString::number(edata.getDataBytes()));
        return;
    }

    QDateTime time = edata.switchCommand.time;
    QLocale l = QLocale(QLocale::English, QLocale::UnitedKingdom);
    stream->writeRawData(l.toString(time,"dd-MMM-yyyy").toUpper().toAscii(), 12);
    stream->writeRawData(l.toString(time,"HH:mm:ss").toAscii(), 9);
    stream->writeRawData(l.toString(time,".z").left(3).toAscii(), 4);

    *stream << (unsigned char)0;
    *stream << (unsigned char)0;
    *stream << (unsigned char)0;
    *stream << (unsigned char)0;

 //   unsigned char flags = 0;
//    if (edata.isCWDirection())
//        flags |= 0x80;
//    flags |= 0x08;
//    if (edata.switchcommand.stepSize==2)
//      flags |= 0x01;

//    if (data.switchCommand.origFileHeader.length()<38)
//        logger()->error("data.switchCommand.origFileHeader has length "+QString::number(data.switchCommand.origFileHeader.length()));
//    else
//        flags = data.switchCommand.origFileHeader[37];
 //    *stream <<  flags;

    *stream << edata.switchCommand.startGain;
    *stream << (unsigned char)0;
    *stream << (unsigned char)20;
    *stream << (unsigned char)9;
    *stream << edata.switchCommand.pulseLength;

    stream->writeRawData(QByteArray(42,0), 42);

    *stream << edata.switchCommand.frequency;

    *stream << (unsigned char)0x10;

    stream->writeRawData(QByteArray(11,0), 11);

    stream->writeRawData(edata.packet, edata.packet.length());

    if (edata.getDataBytes()==0) {
        stream->writeRawData(QByteArray(15,0), 15);
    } else if (edata.getDataBytes()==252) {
        stream->writeRawData(QByteArray(19,0), 19);
    } else if (edata.getDataBytes()==500) {
        stream->writeRawData(QByteArray(27,0), 27);
    }

}

