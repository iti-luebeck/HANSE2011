#include "sonardatasourcefile.h"
#include "QtCore"
#include "module_scanningsonar.h"

SonarDataSourceFile::SonarDataSourceFile(Module_ScanningSonar& parent, QString path)
    : SonarDataSource(parent)
{
    logger = Log4Qt::Logger::logger("SonarFileReader");

    this->file = new QFile(path);
    file->open(QIODevice::ReadOnly);
    if (!file->isReadable()) {
        logger->error("Could not open file " + file->fileName() + " for reading.");
        return;
    }
    this->stream = new QDataStream(file);
}

const SonarReturnData SonarDataSourceFile::getNextPacket()
{
    QDateTime startTime = parent.settings.value("startTime").toDateTime();

    // skip until we are at the startTime
    SonarReturnData p = readPacket();
    while (p.isPacketValid() && startTime>p.dateTime) {
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

    stream->skipRawData(3);
    quint16 totalBytes;
    quint8 nToReadIndex;
    quint16 nToRead;
    char date[12];
    char time[9];
    char hs[4];
    *stream >> nToReadIndex >> totalBytes >> nToRead;
    stream->readRawData(date, 12);
    stream->readRawData(time, 9);
    stream->readRawData(hs, 4);

    QString fullString(date);
    fullString.append(" ");
    fullString.append(time);
    fullString.append(hs);

    // TODO: language must be "C"
    QDateTime dt = QDateTime::fromString(fullString, "dd-MMM-yyyy HH:mm:ss.z");
    logger->trace("rawDateString=" + fullString + "; date="+dt.toString());

    stream->skipRawData(5);

    quint8 startGain;
    *stream >> startGain;

    stream->skipRawData(100 - 39);

    logger->trace("totalBytes=" + QString::number(totalBytes));
    logger->trace("nToReadIndex=" + QString::number(nToReadIndex));
    logger->trace("nToRead=" + QString::number(nToRead));
    logger->trace("startGain=" + QString::number(startGain));
    logger->trace("DateTime=" + dt.toString("ddd MMM d yyyy HH:mm:ss.zzz"));

    char remainingData[totalBytes - 100];
    stream->readRawData(remainingData, totalBytes - 100);
    QByteArray remainingDataArray(remainingData, totalBytes - 100);

    // chop of trailing zero fill
    remainingDataArray.chop(totalBytes - 100 - nToRead);

    logger->trace("Read packet with content " + (QString)remainingDataArray.toHex());

    SonarReturnData d(remainingDataArray, dt);

    d.startGain = startGain;
    return d;
}

bool SonarDataSourceFile::isOpen()
{
    return file && file->isOpen() && stream && !stream->atEnd();
}
