#include "sonardatarecorder.h"
#include "sonarreturndata.h"
#include <QtCore>
#include "module_scanningsonar.h"

SonarDataRecorder::SonarDataRecorder(Module_ScanningSonar& s)
    : sonar(s)
{
    //logger = Log4Qt::Logger::logger("SonarRecorder");

    connect(&s, SIGNAL(newSonarData(SonarReturnData)), this, SLOT(newData(SonarReturnData)));

}

void SonarDataRecorder::newData(const SonarReturnData data)
{
    store(data);
}
