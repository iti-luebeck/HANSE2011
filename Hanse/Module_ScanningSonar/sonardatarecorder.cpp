#include "sonardatarecorder.h"
#include "sonarreturndata.h"
#include <QtCore>
#include "module_scanningsonar.h"

SonarDataRecorder::SonarDataRecorder(Module_ScanningSonar& s, bool formatCSV)
    : sonar(s)
{
    logger = Log4Qt::Logger::logger("SonarRecorder");
    this->formatCSV = formatCSV;

    file = NULL;
    stream = NULL;

}

void SonarDataRecorder::start()
{
    if (file != NULL)
        stop();

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

void SonarDataRecorder::newData(const SonarReturnData data)
{
    if (formatCSV)
        storeCSV(data);
    else
        store852(data);
}

void SonarDataRecorder::storeCSV(const SonarReturnData &data)
{
    if (time.isNull()) {
        time = data.dateTime.time();
    }

    if (!stream || !file) {
        logger->error("Cannot record. stream not open.");
        return;
    }

    time_t t = data.dateTime.toTime_t();

    *stream << data.getRange() << "," << data.getHeadPosition()
            << "," << data.startGain;
        *stream << "," << t;
    for (int i=0; i<data.getEchoData().length(); i++) {
        *stream << "," << (int)data.getEchoData().at(i);
    }
    *stream << "\n";
    stream->flush();
}

void SonarDataRecorder::store852(const SonarReturnData &data)
{
    logger->error("TODO");
}
