#include "module_scanningsonar.h"
#include <QtCore>
#include <stdio.h>
#include "scanningsonar_form.h"
#include "sonarreturndata.h"
#include "sonardatasourceserial.h"
#include "sonardatasourcefile.h"
#include "sonardatacsvrecorder.h"
#include "sonardata852recorder.h"
#include <Module_ThrusterControlLoop/module_thrustercontrolloop.h>

Module_ScanningSonar::Module_ScanningSonar(QString id, Module_ThrusterControlLoop* tcl)
    : RobotModule(id), reader(this)
{
    setDefaultValue("serialPort", "COM1");

    setDefaultValue("range", 50);
    setDefaultValue("gain", 20);
    setDefaultValue("trainAngle", 70);
    setDefaultValue("sectorWidth", 120);
    setDefaultValue("stepSize", 1);
    setDefaultValue("pulseLength", 127);
    setDefaultValue("dataPoints", 25);
    setDefaultValue("switchDelay", 0);
    setDefaultValue("frequency", 0);
    setDefaultValue("readFromFile", false);
    setDefaultValue("recorderFilename", "output.txt");
    setDefaultValue("fileReaderDelay", 100);

    qRegisterMetaType<SonarReturnData>("SonarReturnData");

    this->tcl = tcl;
//    connect(&scanPeriodTimer, SIGNAL(timeout()), this, SLOT(nextScanPeriod()));
    connect(this, SIGNAL(enabled(bool)), this, SLOT(gotEnabledChanged(bool)));

    recorder = NULL;
    source = NULL;

    reset();
}

Module_ScanningSonar::~Module_ScanningSonar()
{
}

Module_ScanningSonar::ThreadedReader::ThreadedReader(Module_ScanningSonar* m)
{
    this->m = m;
    running = true;
}

void Module_ScanningSonar::ThreadedReader::pleaseStop()
{
    running = false;
}

void Module_ScanningSonar::terminate()
{
    RobotModule::terminate();
    logger->debug("Asking Sonar Reading Thread to stop.");
    reader.pleaseStop();
    logger->debug("Waiting for Sonar Reading Thread to terminate.");
    reader.wait();
    logger->debug("Destroying sonar data source and recorder.");
    if (this->source != NULL) {
        delete this->source;
        source = NULL;
    }

    if (recorder != NULL) {
        recorder->stop();
        delete recorder;
        recorder = NULL;
    }
}

void Module_ScanningSonar::ThreadedReader::run(void)
{
    running = true;
    while(running)
    {
        if (!m->getSettings().value("enabled").toBool())
            msleep(500);
    }
}

bool Module_ScanningSonar::doNextScan()
{
    const SonarReturnData d = source->getNextPacket();

    if (!source || !source->isOpen())
        return false;

    if (d.isPacketValid()) {
        setHealthToOk();
        data["currentHeading"] = d.getHeadPosition();
        data["range"] = d.getRange();
        emit newSonarData(d);
        emit dataChanged(this);
        return true;
    } else {
        setHealthToSick("Received bullshit. Dropping packet.");
        return false;
    }
}


void Module_ScanningSonar::reset()
{
    RobotModule::reset();

    logger->debug("Stopping reader.");
    reader.pleaseStop();
    reader.wait();

    logger->debug("Destroying and sonar data source.");
    if (this->source != NULL) {
        this->source->stop();
        delete this->source;
        source = NULL;
    }

    if (recorder != NULL) {
        recorder->stop();
        delete recorder;
        recorder = NULL;
    }

    if (!isEnabled())
        return;

    if (settings.value("enableRecording").toBool()) {
        if (settings.value("formatCSV").toBool())
            recorder = new SonarDataCSVRecorder(*this);
        else
            recorder = new SonarData852Recorder(*this);

        recorder->start();
    }

    if (settings.value("readFromFile").toBool())
        source = new SonarDataSourceFile(*this, settings.value("filename").toString());
    else
        source = new SonarDataSourceSerial(*this);

    logger->debug("Restarting reader.");
    reader.start();
}

QList<RobotModule*> Module_ScanningSonar::getDependencies()
{
    QList<RobotModule*> ret;
    return ret;
}

QWidget* Module_ScanningSonar::createView(QWidget* parent)
{
    return new ScanningSonarForm(this, parent);
}

void Module_ScanningSonar::gotEnabledChanged(bool state)
{
    if (!state)
        reset();
}
