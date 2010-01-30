#include "sonardatarecorder.h"
#include <sonarreturndata.h>
#include "QtCore"
#include <module_scanningsonar.h>

SonarDataRecorder::SonarDataRecorder(Module_ScanningSonar& s)
    : sonar(s)
{
    logger = Log4Qt::Logger::logger("SonarRecorder");
}

void SonarDataRecorder::start()
{
    file = new QFile(sonar.getSettings().value("recorderFilename").toString());
    if (file->open(QFile::WriteOnly | QFile::Truncate)) {
        stream = new QTextStream(file);
    } else {
        logger->error("Could not open file "+file->fileName());
    }
}

void SonarDataRecorder::stop()
{
    file->close();
}

void SonarDataRecorder::newData(SonarReturnData data)
{
    logger->debug("Writing packet to file");
    *stream << data.getRange() << "," << data.getHeadPosition();
    for (int i=0; i<data.getEchoData().length(); i++) {
        *stream << "," << (int)data.getEchoData().at(i);
    }
    *stream << "\n";
    stream->flush();
}
