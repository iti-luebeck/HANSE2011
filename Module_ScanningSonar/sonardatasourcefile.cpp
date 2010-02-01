#include "sonardatasourcefile.h"
#include "QtCore"
#include "module_scanningsonar.h"

class SleeperThread : public QThread
{
public:
static void msleep(unsigned long msecs)
{
QThread::msleep(msecs);
}
};

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

SonarReturnData* SonarDataSourceFile::getNextPacket()
{
    if (!stream) {
        logger->error("Stream not open!");
        SleeperThread::msleep(parent.getSettings().value("fileReaderDelay").toInt());
        return NULL;
    }

    if (stream->atEnd()) {
        logger->error("Reached end of file!");
        SleeperThread::msleep(1000);
        return NULL;
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
    fullString.append(time);
    fullString.append(hs);

    QDateTime dt = QDateTime::fromString(fullString, "dd-MMM-yyyyHH:mm:ss.z");

    stream->skipRawData(100 - 33);

    logger->trace("totalBytes=" + QString::number(totalBytes));
    logger->trace("nToReadIndex=" + QString::number(nToReadIndex));
    logger->trace("nToRead=" + QString::number(nToRead));
    logger->trace("DateTime=" + dt.toString("ddd MMM d yyyy HH:mm:ss.zzz"));

    char remainingData[totalBytes - 100];
    stream->readRawData(remainingData, totalBytes - 100);
    QByteArray remainingDataArray(remainingData, totalBytes - 100);

    // chop of trailing zero fill
    remainingDataArray.chop(totalBytes - 100 - nToRead);

    logger->trace("Read packet with content " + (QString)remainingDataArray.toHex());

    SleeperThread::msleep(parent.getSettings().value("fileReaderDelay").toInt());
    return new SonarReturnData(remainingDataArray, dt);
}

