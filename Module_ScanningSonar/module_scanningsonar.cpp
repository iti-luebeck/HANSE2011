#include "module_scanningsonar.h"
#include <QtCore>
#include <stdio.h>
#include "form.h"
#include <sonarreturndata.h>
#include <sonardatasourceserial.h>
#include <sonardatasourcefile.h>
#include <sonardatarecorder.h>

Module_ScanningSonar::Module_ScanningSonar(QString id)
    : reader(this), RobotModule(id)
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
    recorder = new SonarDataRecorder(*this);
    recorder->start();
    connect(this, SIGNAL(newSonarData(SonarReturnData)), recorder, SLOT(newData(SonarReturnData)));

    if (settings.value("readFromFile").toBool())
        source = new SonarDataSourceFile(*this, settings.value("filename").toString());
    else
        source = new SonarDataSourceSerial(*this, settings.value("serialPort").toString());

    reader.start();
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
    logger->debug("Finished.");
    recorder->stop();
}

void Module_ScanningSonar::ThreadedReader::run(void)
{
    running = true;
    while(running)
    {
        if (m->getSettings().value("enabled").toBool())
            m->doNextScan();
        else
            msleep(500);
    }
}

void Module_ScanningSonar::doNextScan()
{
    SonarReturnData* d = source->getNextPacket();

    if (d && d->isPacketValid()) {
        retData.append(d);
        setHealthToOk();
        data["currentHeading"] = d->getHeadPosition();
        data["range"] = d->getRange();
        emit newSonarData(*d);
        emit dataChanged(this);
    } else {
        setHealthToSick("Received bullshit. Dropping packet.");
    }
}


void Module_ScanningSonar::reset()
{
    logger->debug("Stopping reader temporarily.");
    reader.pleaseStop();
    reader.wait();

    logger->debug("Destroying and recreating sonar data source.");
    delete this->source;
    if (settings.value("readFromFile").toBool())
        source = new SonarDataSourceFile(*this, settings.value("filename").toString());
    else
        source = new SonarDataSourceSerial(*this, settings.value("serialPort").toString());

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
    return new Form(this, parent);
}

