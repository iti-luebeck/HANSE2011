#include "module_echosounder.h"
#include <QtCore>
#include <stdio.h>
#include "echosounderform.h"
#include "echoreturndata.h"
#include "echodatasourceserial.h"
#include "echodatasourcefile.h"
#include "echodatacsvrecorder.h"
#include "echodata852recorder.h"
#include <Module_ThrusterControlLoop/module_thrustercontrolloop.h>
#include <Module_Simulation/module_simulation.h>

Module_EchoSounder::Module_EchoSounder(QString id, Module_Simulation *sim)
    : RobotModule(id), reader(this, sim)
{
    setDefaultValue("serialPort", "COM9");
    setDefaultValue("range", 5);
    setDefaultValue("gain", 20);
    setDefaultValue("pulseLength", 127);
    setDefaultValue("profileMinRange", 0);
    setDefaultValue("dataPoints", 25);
    setDefaultValue("profile", 0);
    setDefaultValue("switchDelay", 0);
    setDefaultValue("frequency", 0);
    setDefaultValue("readFromFile", false);
    setDefaultValue("recordFile", "output.txt");
    setDefaultValue("fileReaderDelay", 100);

    qRegisterMetaType<EchoReturnData>("EchoReturnData");

    this->sim = sim;

    recorder = NULL;
    source = NULL;

    //reset();
}
Module_EchoSounder::~Module_EchoSounder(){
}

void Module_EchoSounder::init()
{
    timer.moveToThread(this);
    connect(this, SIGNAL(enabled(bool)), this, SLOT(gotEnabledChanged(bool)));
    connect(sim,SIGNAL(newEchoData(EchoReturnData)), this, SLOT(refreshSimData(EchoReturnData)));
    connect(this,SIGNAL(requestEchoSignal()), sim, SLOT(requestEchoSlot()));
    reset();
}

Module_EchoSounder::ThreadedReader::ThreadedReader(Module_EchoSounder *m, Module_Simulation *sim)
{
    this->m = m;
    this->sim = sim;
    running = true;
}

void Module_EchoSounder::ThreadedReader::pleaseStop()
{
    running = false;
}

void Module_EchoSounder::terminate(){

    logger->debug("Asking Echo Reading Thread to stop.");
    reader.pleaseStop();
    logger->debug("Waiting for Echo Reading Thread to terminate.");
    reader.wait();
    logger->debug("Destroying echo data source and recorder.");
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

void Module_EchoSounder::ThreadedReader::run(void){
    running = true;
    while(running)
    {

        if (m->getSettingsValue("enabled").toBool()){// || !m->doNextScan()){
            m->doNextScan();
            msleep(500);
        }
    }
}

void Module_EchoSounder::emitEchoSignal()
{
    emit requestEchoSignal();
}

void Module_EchoSounder::refreshSimData(EchoReturnData dat){
    addData("range", dat.getRange());

    emit newEchoData(dat);
    emit dataChanged(this);
}

bool Module_EchoSounder::doNextScan(){
//    if(sim->isEnabled()){
//        emit requestEchoSignal();
  //      return true;
 //   }else{
//        const EchoReturnData d;

        const EchoReturnData d = source->getNextPacket();

        if(!source || !source->isOpen()){
            logger->error("Nein das source ist kaputt");
            return false;
        }
        if(d.isPacketValid()){
            setHealthToOk();
            addData("range", d.getRange());
            emit newEchoData(d);
            emit dataChanged(this);
            return true;
        } else {
            setHealthToSick("Received §&%§%§=§$ Dropping packet.");
            return false;
        }
//    }
}

void Module_EchoSounder::reset(){
    RobotModule::reset();

    logger->debug("Stopping reader.");
    reader.pleaseStop();
    reader.wait();

    logger->debug("Destroying and echo data source.");
    if (this->source != NULL) {
        this->source->stop();
        delete this->source;
        source = NULL;
    }
    logger->debug("source tot");
    if (recorder != NULL) {
        recorder->stop();
        delete recorder;
        recorder = NULL;
    }
    logger->debug("recorder tot");
    if (!isEnabled()){
        return;
    }
    logger->debug("starting recorder");
    if (getSettingsValue("enableRecording").toBool()) {
        if (getSettingsValue("formatCSV").toBool())
            recorder = new EchoDataCSVRecorder(*this);
        else
            recorder = new EchoData852Recorder(*this);

        recorder->start();
    }

//    reader.start();
//    if (sim->isEnabled())
//        return;
     logger->debug("open source");
    if (getSettingsValue("readFromFile").toBool())
    {
        logger->debug("starting source");
        source = new EchoDataSourceFile(*this, getSettingsValue("filename").toString());
    }
    else
        source = new EchoDataSourceSerial(*this);

    logger->debug("Restarting reader.");
    reader.start();
}

// Klappt das so?!
//EchoReturnData Module_EchoSounder::getScan(){
//    if(doNextScan() == true){
//        return d;
//    }else{
//        return NULL;
//    }
//}

QList<RobotModule*> Module_EchoSounder::getDependencies()
{
    QList<RobotModule*> ret;
    ret.append(sim);
    return ret;
}

QWidget* Module_EchoSounder::createView(QWidget *parent)
{
    return new EchoSounderForm(this, parent);
}

void Module_EchoSounder::gotEnabledChanged(bool state)
{
    if(!state){
        reset();
    }
}
