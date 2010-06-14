#include "sonardatacsvrecorder.h"
#include <Framework/dataloghelper.h>
#include "sonardatarecorder.h"
#include "sonarreturndata.h"
#include <QtCore>
#include "module_scanningsonar.h"

SonarDataCSVRecorder::SonarDataCSVRecorder(Module_ScanningSonar& s)
    : SonarDataRecorder(s)
{
    file = NULL;
    stream = NULL;
}

void SonarDataCSVRecorder::start()
{
    if (file != NULL)
        stop();

    file = new QFile(DataLogHelper::getLogDir()+"sonarlog.csv");
    if (file->open(QFile::WriteOnly | QFile::Truncate)) {
        stream = new QTextStream(file);
    } else {
        logger()->error("Could not open file "+file->fileName());
    }
}

void SonarDataCSVRecorder::stop()
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

void SonarDataCSVRecorder::store(const SonarReturnData &data)
{
    if (!stream || !file) {
        logger()->error("Cannot record. stream not open.");
        return;
    }

    time_t t = data.switchCommand.time.toTime_t();

    *stream << data.getRange() << "," << data.getHeadPosition()
            << "," << data.switchCommand.startGain;
        *stream << "," << t;
    for (int i=0; i<data.getEchoData().length(); i++) {
        *stream << "," << (int)data.getEchoData().at(i);
    }
    *stream << "\n";
    stream->flush();
}
