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
    logger = Log4Qt::Logger::logger("sonar");

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
        return NULL;
    }

    stream->skipRawData(100);

    int expectedLength;
    if (parent.getSettings().value("dataPoints", 50).toInt())
        expectedLength = 513;
    else
        expectedLength = 265;

    char a[expectedLength];
    int r = stream->readRawData(a, expectedLength);
    QByteArray array(a,expectedLength);

    return new SonarReturnData(array);
}

