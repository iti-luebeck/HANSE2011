#include "module_scanningsonar.h"
#include <QtCore>
#include <stdio.h>
#include "scanningsonar_form.h"
#include "sonarreturndata.h"
#include "sonardatasourceserial.h"
#include "sonardatasourcefile.h"
#include "sonardatacsvrecorder.h"
#include "sonardata852recorder.h"
#include "sonardatasourcerivas.h"
#include <Module_Simulation/module_simulation.h>
#include <Module_XsensMTi/module_xsensmti.h>

Module_ScanningSonar::Module_ScanningSonar(QString id, Module_Simulation *sim, Module_XsensMTi *mti)
    : RobotModule(id)
{
    this->sim = sim;
    this->mti = mti;
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
    setDefaultValue("enableRecording", true);
    setDefaultValue("formatCSV", false);
    setDefaultValue("counter852", 0);
    setDefaultValue("counterCSV", 0);

    qRegisterMetaType<SonarReturnData>("SonarReturnData");
    recorder = NULL;
    source = NULL;
    sonarTime = 0;
    timer.moveToThread(this);
}

Module_ScanningSonar::~Module_ScanningSonar()
{
}

void Module_ScanningSonar::init()
{
    connect(&timer,SIGNAL(timeout()), this, SLOT(doNextScan()));
    connect(this, SIGNAL(enabled(bool)), this, SLOT(gotEnabledChanged(bool)));

    /* connect simulation */
    connect(sim,SIGNAL(newSonarData(SonarReturnData)), this, SLOT(refreshSimData(SonarReturnData)));
    connect(this,SIGNAL(requestSonarSignal()),sim,SLOT(requestSonarSlot()));
    reset();

}

void Module_ScanningSonar::terminate()
{
    QTimer::singleShot(0,&timer,SLOT(stop()));
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
    RobotModule::terminate();
}

void Module_ScanningSonar::refreshSimData(SonarReturnData data)
{
    addData("heading",data.getHeadPosition());
    addData("range",data.getRange());
    emit newSonarData(data);
    emit dataChanged(this);
}

bool Module_ScanningSonar::doNextScan()
{
    if(!this->isEnabled())
    {
        timer.stop();
        return false;
    }

    if(sim->isEnabled())
    {
        emit requestSonarSignal();
        return true;
    }
    else
    {

        if (!source || !source->isOpen()) {
            setHealthToSick("source not open.");
            msleep(100);
            return false;
        }

        const SonarReturnData d = source->getNextPacket();
        sonarTime = source->sonarTime;

        if (d.isPacketValid()) {
            setHealthToOk();
            addData("heading", d.getHeadPosition());
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
    if (!isEnabled())
        return;

    RobotModule::reset();

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


    if (getSettingsValue("enableRecording").toBool()) {
        if (getSettingsValue("formatCSV").toBool())
            recorder = new SonarDataCSVRecorder(*this);
        else
            recorder = new SonarData852Recorder(*this);

        recorder->start();
    }

    if (sim->isEnabled())
    {
        // XXX: this is inadequate for small sonar ranges
        timer.setInterval(100);
        timer.start();

    } else {

        if (getSettingsValue("readFromFile").toBool())
        {
            if (getSettingsValue("is rivas", false).toBool()) {
                source = new SonarDataSourceRivas(*this, this->mti, getSettingsValue("filename").toString(), getSettingsValue("mti filename").toString());
            } else {
                source = new SonarDataSourceFile(*this, getSettingsValue("filename").toString());
            }
            if(!source->isOpen())
            {
                logger->error("source not opened");
                source = NULL;
            }
        }
        else {
            source = new SonarDataSourceSerial(*this);
        }

        logger->debug("Restarting reader.");
        if(source != NULL && source->isOpen())
        {
           timer.setInterval(0); // timerSlot will block
           timer.start();
       }
    }
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
