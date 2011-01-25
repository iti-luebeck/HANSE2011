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
#include <Module_Simulation/module_simulation.h>
Module_ScanningSonar::Module_ScanningSonar(QString id, Module_ThrusterControlLoop *tcl, Module_Simulation *sim)
    : RobotModule_MT(id), reader(this)
{
    this->sim = sim;
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
    timer = new QTimer();
    connect(timer,SIGNAL(timeout()), this, SLOT(doNextScan()));
    connect(this, SIGNAL(enabled(bool)), this, SLOT(gotEnabledChanged(bool)));

    /* connect simulation */
    connect(sim,SIGNAL(newSonarData(SonarReturnData)), this, SLOT(refreshSimData(SonarReturnData)));
    connect(this,SIGNAL(requestSonarSignal()),sim,SLOT(requestSonarSlot()));

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
//    RobotModule::terminate();
    timer->stop();
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
    RobotModule_MT::terminate();
}

void Module_ScanningSonar::ThreadedReader::run(void)
{
    running = true;
    while(running)
    {
        if (m->getSettingsValue("enabled").toBool()){// || !m->doNextScan()){
            m->doNextScan();
            msleep(500);
        }
    }
}

void Module_ScanningSonar::refreshSimData(SonarReturnData data)
{
    addData("currentHeading",data.getHeadPosition());
    addData("range",data.getRange());
    emit newSonarData(data);
    emit dataChanged(this);
}

bool Module_ScanningSonar::doNextScan()
{

    if(sim->isEnabled())
    {
        emit requestSonarSignal();
        return true;
    }
    else
    {
        const SonarReturnData d = source->getNextPacket();

        if (!source || !source->isOpen())
            return false;

        if (d.isPacketValid()) {
            setHealthToOk();
            addData("currentHeading", d.getHeadPosition());
            addData("range", d.getRange());
            emit newSonarData(d);
            emit dataChanged(this);
            return true;
        } else {
            setHealthToSick("Received bullshit. Dropping packet.");
            return false;
        }
    }
}


void Module_ScanningSonar::reset()
{
    RobotModule::reset();

    logger->debug("Stopping reader.");
//    reader.pleaseStop();
//    reader.wait();

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


    if (getSettingsValue("enableRecording").toBool()) {
        if (getSettingsValue("formatCSV").toBool())
            recorder = new SonarDataCSVRecorder(*this);
        else
            recorder = new SonarData852Recorder(*this);

        recorder->start();
    }

//    reader.start();

    if (sim->isEnabled())
    {
        QTimer::singleShot(0, timer, SLOT(start()));
        return;
    }

    if (getSettingsValue("readFromFile").toBool())
    {
        source = new SonarDataSourceFile(*this, getSettingsValue("filename").toString());
        if(!source->isOpen())
        {
            logger->error("source not opened");
            source = NULL;
        }
    }
    else
        source = new SonarDataSourceSerial(*this);

    logger->debug("Restarting reader.");
    timer->setInterval(500);
    if(source != NULL)
    {
       QTimer::singleShot(0, timer, SLOT(start()));
       setHealthToOk();
   }
    else
    {
        setHealthToSick("source is null");
    }
//    reader.start();
}

QList<RobotModule*> Module_ScanningSonar::getDependencies()
{
    QList<RobotModule*> ret;
    ret.append(sim);
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
