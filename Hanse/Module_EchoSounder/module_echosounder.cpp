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
    : RobotModule(id)
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

    setDefaultValue("threshold", 30);
    setDefaultValue("averageWindow", 12);
    setDefaultValue("count", 0);

    setDefaultValue("scanTimer",100);
    setDefaultValue("calcFactor",0.3);
    setDefaultValue("formatCSV", false);
    setDefaultValue("enableRecording", true);

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
    timer.moveToThread(this);

    //reset();
}

void Module_EchoSounder::init()
{
    //timer.moveToThread(this);
    connect(this, SIGNAL(enabled(bool)), this, SLOT(gotEnabledChanged(bool)));
    /* connect simulation */
    connect(sim,SIGNAL(newSonarSideData(EchoReturnData)), this, SLOT(refreshSimData(EchoReturnData)));
    connect(this,SIGNAL(requestSonarSideSignal()),sim,SLOT(requestSonarSideSlot()));

    connect(&timer,SIGNAL(timeout()), this, SLOT(doNextScan()));

    reset();
}

void Module_EchoSounder::terminate(){

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

void Module_EchoSounder::refreshSimData(EchoReturnData dat){
    addData("range", dat.getRange());
    emit newEchoData(dat);
    scanningOutput(dat);
    emit dataChanged(this);
}

bool Module_EchoSounder::doNextScan(){

    if(!this->isEnabled())
    {
        timer.stop();
        return false;
    }

    if(sim->isEnabled()){
        emit requestSonarSideSignal();
        return true;
    }else{

        if(!source || !source->isOpen()){
            setHealthToSick("source not open.");
            msleep(100);
            emit dataError();
            return false;
        }

        EchoReturnData d = source->getNextPacket();

        if(d.isPacketValid()){
            setHealthToOk();
            addData("range", d.getRange());
            emit newEchoData(d);
            scanningOutput(d);
            emit dataChanged(this);
            return true;
        } else {
            setHealthToSick("Received bullshit. Dropping packet.");
            emit dataError();
            qDebug("Received bullshit. Dropping packet.");
            return false;
        }
    }
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

    float aktMax = 0.0;
    char c;

    // 5 Datenarrays werden für die Distanzberechnung gefüllt
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
        }

        avgDistance = 0.0;
        // Summe aus allen 5 Datenarrays, Durchschnitt pro Datenwert
        for(int i = 0; i < 5; i++){
            for (int j = 0; j < dataLength; j++){
                avgSig[j] = avgSig[j]+fewSigAvg[i][j];
            }
        }

        for(int i=0; i<dataLength;i++){
            avgSig[i] = (avgSig[i]/5);
        }

        for(int i=0; i<dataLength;i++){
            if(avgSig[i]<threshold){
                avgSig[i]=0;
            }
        }

        for (int r = 0; r < 252; r++){

            if(aktMax < avgSig[r]){
                aktMax = avgSig[r];
            }
        }

        // Prüfen, ob die naechsten X Datenwerte den Schwellwert überschreiten
        for(int x = 0; x < dataLength-averageWindow; x++){
            for(int y = x; y<x+averageWindow-1; y++){
                avgFilter = avgFilter+avgSig[y];

            }

            calcFactor = getSettingsValue("calcFactor").toFloat();

            float a = ((calcFactor)*(float)averageWindow * aktMax);

            if(avgFilter>a){
                avgDistance = (x+3)/einheit;
                // Berechnung abgeschlossen, also raus hier!
                break;
            } else {
                avgDistance = 0.0;
                avgFilter = 0.0;
            }
        }
    }
    addData("avgDistance",avgDistance);
    addData("count",count);
    addData("averageWindow",averageWindow);
    emit newEchoUiData(avgDistance,averageWindow);
    emit newWallBehaviourData(data, avgDistance);
    emit dataChanged(this);

    // Zaehler um zu gucken, wie viele Messungen schon verarbeitet wurden
    count++;

}

float Module_EchoSounder::getAvgCm(){
    return avgDistance;
}

void Module_EchoSounder::reset(){
    RobotModule::reset();

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

    if (sim->isEnabled())
    {
        timer.setInterval(500);
        timer.start();

    } else {
        logger->debug("open source");
        if (getSettingsValue("readFromFile").toBool())
        {
            logger->debug("starting source");
            source = new EchoDataSourceFile(*this, getSettingsValue("filename").toString());
        }
        else
            source = new EchoDataSourceSerial(*this);

        if(source->isOpen())
        {
        // read as fast as possible from the echo sounder
        timer.setInterval(0);
        timer.start();
    }
    }
}


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
