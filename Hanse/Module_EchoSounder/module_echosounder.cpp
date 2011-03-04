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

    setDefaultValue("teshold", 30);
    setDefaultValue("averageWindow", 12);
    setDefaultValue("count", 0);

    qRegisterMetaType<EchoReturnData>("EchoReturnData");

    this->sim = sim;

    recorder = NULL;
    source = NULL;

    for(int i = 0; i < 4; i++){
        for (int j = 0; j < 252; j++){
            fewSigAvg[i][j] = 0;
            avgSig[j] = 0;
        }
    }
    count = 0;

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
    emit newEchoData(dat);
    emit dataChanged(this);
}

bool Module_EchoSounder::doNextScan(){
//    if(sim->isEnabled()){
//        emit requestEchoSignal();
  //      return true;
 //   }else{
//        const EchoReturnData d;

        EchoReturnData d = source->getNextPacket();

        if(!source || !source->isOpen()){
            logger->error("Nein das source ist kaputt");
            return false;
        }
        if(d.isPacketValid()){
            setHealthToOk();
            addData("range", d.getRange());
            emit newEchoData(d);
            scanningOutput(d);
            emit dataChanged(this);
            return true;
        } else {
            setHealthToSick("Received §&%§%§=§$ Dropping packet.");
            return false;
        }
//    }
}

void Module_EchoSounder::scanningOutput(const EchoReturnData data){

    float dataLength = data.getEchoData().length();
    // Gewünschte Ausgabeeinheit
    float einheit = dataLength/data.getRange();

    // Variable die Angibt, wie viele Datenwerte gleichzeitig angeguckt werden,
    // um über einen Schwellwert zu kommen.
    averageWindow = getSettingsValue("averageWindow").toInt();

    // Der eben genannte Schwellwert, optimaler Wert muss noch gesucht werden!
    threshold = getSettingsValue("threshold").toInt();

    // Berechnete Summe von avgTest Datenwerte
    float avgFilter = 0.0;

    // Berechneter Durchschnittsabstand


    float temp = 0.0;

    float aktMax = 0.0;
    char c;

    if(count%5==4){
        for (int i = 0; i < dataLength; i++) {
            c = data.getEchoData()[i];
            if(c < threshold){
                fewSigAvg[0][i] = 0;
            }else{
                fewSigAvg[0][i] = data.getEchoData()[i];
            }
        }
    } else if(count%5==3){
        for (int i = 0; i < dataLength; i++) {
            c = data.getEchoData()[i];
            if(c < threshold){
                fewSigAvg[1][i] = 0;
            }else{
                fewSigAvg[1][i] = data.getEchoData()[i];
            }
        }
    } else if(count%5==2){
        for (int i = 0; i < dataLength; i++) {
            c = data.getEchoData()[i];
            if(c < threshold){
                fewSigAvg[2][i] = 0;
            }else{
                fewSigAvg[2][i] = data.getEchoData()[i];
            }
        }
    } else if(count%5==1){
        for (int i = 0; i < dataLength; i++) {
            c = data.getEchoData()[i];
            if(c < threshold){
                fewSigAvg[3][i] = 0;
            }else{
                fewSigAvg[3][i] = data.getEchoData()[i];
            }
        }
    } else if(count%5==0){
        for (int i = 0; i < dataLength; i++) {
            c = data.getEchoData()[i];
            if(c < threshold){
                fewSigAvg[4][i] = 0;
            }else{
                fewSigAvg[4][i] = data.getEchoData()[i];
            }
        }
    }

    if(count!=0){
        if(count%5==0){
            count = 0;
            avgDistance = 0.0;
            // Summe aus allen 5 Datenarrays, Durchschnitt pro Datenwert
            for(int i = 0; i < 5; i++){
                for (int j = 0; j < dataLength; j++){
                    temp = avgSig[j]+fewSigAvg[i][j];
                    avgSig[j] = (temp/5);
                }
            }

            for (int r = 0; r < 252; r++){
                if(aktMax < avgSig[r]){
                    aktMax = avgSig[r];
                }
            }

            // Prüfen, ob die nächsten X Datenwerte den Schwellwert überschreiten
            for(int x = 0; x < dataLength-averageWindow; x++){
                for(int y = x; y<x+averageWindow; y++){
                    avgFilter = avgFilter+avgSig[y];
                }

                if(avgFilter>((0.875)*(float)averageWindow * aktMax)){

                    avgDistance = (x+3)/einheit;
                    emit newEchoUiData(avgDistance,averageWindow);

                    // Berechnung abgeschlossen, also raus hier!
                    break;
                }
            }

        }
    }
    addData("avgDistance",avgDistance);
    addData("count",count);
    addData("averageWindow",averageWindow);
    emit newWallBehaviourData(data, avgDistance);
    emit dataChanged(this);


    // Zähler um zu gucken, wie viele Messungen schon verarbeitet wurden
    count++;


}

float Module_EchoSounder::getAvgCm(){
    return avgDistance;
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
