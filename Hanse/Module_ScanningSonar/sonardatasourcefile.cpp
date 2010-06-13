#include "sonardatasourcefile.h"
#include "QtCore"
#include "module_scanningsonar.h"
#include "sonarswitchcommand.h"

SonarDataSourceFile::SonarDataSourceFile(Module_ScanningSonar& parent, QString path)
    : SonarDataSource(parent)
{
    logger = Log4Qt::Logger::logger("SonarFileReader");

    this->stream = NULL;
    this->file = new QFile(path);
    if (!file->open(QIODevice::ReadOnly)) {
        logger->error("Could not open file " + file->fileName() + " for reading.");
        delete file;
        file = NULL;
        return;
    }

    this->stream = new QDataStream(file);
}

const SonarReturnData SonarDataSourceFile::getNextPacket()
{
    QDateTime startTime = parent.settings.value("startTime").toDateTime();

    // skip until we are at the startTime
    SonarReturnData p = readPacket();
    // TODO: will block until the file is finished
    while (p.isPacketValid() && startTime>p.switchCommand.time) {
        p = readPacket();
    }

    parent.msleep(parent.getSettings().value("fileReaderDelay").toInt());

    return p;
}

SonarReturnData SonarDataSourceFile::readPacket()
{
    if (!stream) {
        logger->error("Stream not open!");
        parent.msleep(parent.getSettings().value("fileReaderDelay").toInt());
        SonarReturnData inv;
        return inv;
    }

    if (stream->atEnd()) {
        logger->error("Reached end of file!");
        parent.msleep(1000);
        SonarReturnData inv;
        return inv;
    }


    char header[100];
    stream->readRawData(header, 100);

    SonarSwitchCommand cmd(QByteArray(header,100));

    quint16 totalBytes = cmd.totalBytes;

    char remainingData[totalBytes - 100];
    stream->readRawData(remainingData, totalBytes - 100);
    QByteArray remainingDataArray(remainingData, totalBytes - 100);

    // chop of trailing zero fill
    remainingDataArray.chop(totalBytes - 100 - cmd.nToRead);

    logger->trace("Read packet with content " + (QString)remainingDataArray.toHex());

    SonarReturnData d(cmd, remainingDataArray);
    return d;
}

bool SonarDataSourceFile::isOpen()
{
    return file && file->isOpen() && stream && !stream->atEnd();
}
