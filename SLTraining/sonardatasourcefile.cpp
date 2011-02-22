#include "sonardatasourcefile.h"
#include "QtCore"
#include "sonarswitchcommand.h"

class sotoSleep : public QThread
{
        public:
                static void sleep(unsigned long secs)
                {
                        QThread::sleep(secs);
                }
                static void msleep(unsigned long msecs)
                {
                        QThread::msleep(msecs);
                }
                static void usleep(unsigned long usecs)
                {
                        QThread::usleep(usecs);
                }
};


SonarDataSourceFile::SonarDataSourceFile(QObject *parent, QString path)
{
    this->stream = NULL;
    this->file = new QFile(path);
    if (!file->open(QIODevice::ReadOnly)) {
        qDebug() << "Could not open file " << file->fileName() << " for reading.";
        delete file;
        file = NULL;
        return;
    }

    this->stream = new QDataStream(file);
}

const SonarReturnData SonarDataSourceFile::getNextPacket()
{
    // skip until we are at the startTime
    SonarReturnData p = readPacket();
    // TODO: will block until the file is finished
    while (p.isPacketValid() && startTime>p.switchCommand.time) {
        p = readPacket();
    }

    sotoSleep::msleep(fileReaderDelay);

    return p;
}

SonarReturnData SonarDataSourceFile::readPacket()
{
    if (!stream) {
        qDebug() << "Stream not open!";
        sotoSleep::msleep(fileReaderDelay);
        SonarReturnData inv;
        return inv;
    }

    if (stream->atEnd()) {
        qDebug() << "EOF";
        sotoSleep::msleep(1000);
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

//    qDebug() << "Read packet with content " << (QString)remainingDataArray.toHex();

    SonarReturnData d(cmd, remainingDataArray);
    return d;
}

bool SonarDataSourceFile::isOpen()
{
    return file && file->isOpen() && stream && !stream->atEnd();
}

SonarDataSourceFile::~SonarDataSourceFile()
{

}

void SonarDataSourceFile::stop()
{
    file->close();
}
