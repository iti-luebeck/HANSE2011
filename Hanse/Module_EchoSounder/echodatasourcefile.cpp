#include "echodatasourcefile.h"
#include "QtCore"
#include "module_echosounder.h"
#include "echoswitchcommand.h"

EchoDataSourceFile::EchoDataSourceFile(Module_EchoSounder& parent, QString path)
    : EchoDataSource(parent)
{
    logger = Log4Qt::Logger::logger("EchoFileReader");

    this->stream = NULL;
    this->file = new QFile(path);
    if(!file->open(QIODevice::ReadOnly)){
        logger->error("Could not open file "+ file->fileName() + " for reading.");
        delete file;
        file = NULL;
        return;
    }

    this->stream = new QDataStream(file);
}

const EchoReturnData EchoDataSourceFile::getNextPacket(){
    QDateTime startTime = parent.getSettingsValue("startTime").toDateTime();

    // skip until we are at the startTime
    EchoReturnData p = readPacket();
    // TODO: will block until the file is finished
    while(p.isPacketValid() && startTime>p.switchCommand.time){
        p = readPacket();
    }

    parent.msleep(parent.getSettingsValue("fileReaderDelay").toInt());

    return p;
}

EchoReturnData EchoDataSourceFile::readPacket(){
    if(!stream){
        logger->error("Stream not open!");
        parent.msleep(parent.getSettingsValue("fileReaderDelay").toInt());
        EchoReturnData inv;
        return inv;
    }
    if(stream->atEnd()){
        logger->error("Reached end of file!");
        parent.msleep(1000);
        EchoReturnData inv;
        return inv;
    }

    char header[100];
    stream->readRawData(header, 100);

    EchoSwitchCommand cmd(QByteArray(header, 100));
    quint16 totalBytes = cmd.totalBytes;

    char remainingData[totalBytes - 100];
    stream->readRawData(remainingData, totalBytes - 100);
    QByteArray remainingDataArray(remainingData, totalBytes - 100);

    // chop of trailing zero fill
    remainingDataArray.chop(totalBytes - 100 - cmd.nToRead);

    logger->trace("Read packet with content " + (QString)remainingDataArray.toHex());

    EchoReturnData d(cmd, remainingDataArray);
    return d;
}

bool EchoDataSourceFile::isOpen(){
    return file && file->isOpen() && stream && !stream->atEnd();
}

EchoDataSourceFile::~EchoDataSourceFile(){

}

void EchoDataSourceFile::stop(){
    file->close();
}

