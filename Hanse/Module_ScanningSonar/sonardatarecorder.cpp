#include "sonardatarecorder.h"
#include "sonarreturndata.h"
#include <QtCore>
#include "module_scanningsonar.h"

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
    time = QTime();
}

void SonarDataRecorder::stop()
{
    file->close();
}

void SonarDataRecorder::newData(const SonarReturnData data)
{
    if (time.isNull()) {
        time = data.dateTime.time();
    }
    if (sonar.getSettings().value("enableRecording").toBool()) {

        time_t t = data.dateTime.toTime_t();
        //int t = time.secsTo(data.dateTime.time());

        logger->debug("Writing packet to file");
        logger->trace("bla:startGain=" + QString::number(data.startGain));
        *stream << data.getRange() << "," << data.getHeadPosition()
                << "," << data.startGain;
            *stream << "," << t;
        for (int i=0; i<data.getEchoData().length(); i++) {
            *stream << "," << (int)data.getEchoData().at(i);
        }
        *stream << "\n";
        stream->flush();
    }
}
