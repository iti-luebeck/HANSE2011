#include "module_scanningsonar.h"
#include <QWidget>
#include <QMessageBox>
#include <stdio.h>
#include "form.h"

class SleeperThread : public QThread
{
public:
static void msleep(unsigned long msecs)
{
QThread::msleep(msecs);
}
};


Module_ScanningSonar::Module_ScanningSonar(QString id)
    : reader(this), RobotModule(id)
{
    configurePort();

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
    logger->debug("Asking Sonar Reading Thread to stop.");
    reader.pleaseStop();
    logger->debug("Waiting for Sonar Reading Thread to terminate.");
    reader.wait();
    logger->debug("Closing serial port.");
    port.close();
    logger->debug("Finished.");
}

void Module_ScanningSonar::ThreadedReader::run(void)
{
    while(running)
    {
        m->doNextScan();
    }
}

void Module_ScanningSonar::doNextScan()
{
    logger->debug("Sending switch data command.");
    QByteArray sendArray = buildSwitchDataCommand();
    logger->debug("Sending: " + QString(sendArray.toHex()));
    port.write(sendArray);

    QByteArray retData;
    SleeperThread t;

    int expectedLength;
    if (settings.value("dataPoints", 50).toInt())
        expectedLength = 513;
    else
        expectedLength = 265;

    // TODO: add some kind of timeout to handle communication errors
    int timeout = 1000;
//    while(retData.length() < expectedLength && timeout>0) {
    while(timeout>0 && (retData.length()==0 || retData[retData.length()-1] != 0xFC)) {
        QByteArray ret = port.read(expectedLength - retData.length());
        retData.append(ret);
        t.msleep(5); timeout -= 5;
        //logger->debug("Partial receive: " + QString(ret.toHex()));
    }
    logger->debug("Received in total: " + QString(retData.toHex()));

    // TODO: parse data
    SonarReturnData* d = new SonarReturnData(retData);

    if (d->isPacketValid()) {
        data.append(d);
        emit newSonarData();
    }

}

void Module_ScanningSonar::configurePort()
{
    // TODO: something missing!
    port.setPortName(settings.value("port", "COM1").toString());
    port.setBaudRate(BAUD115200);
    port.setDataBits(DATA_8);
    port.setStopBits(STOP_1);
    port.setParity(PAR_NONE);
    port.setFlowControl(FLOW_OFF);
    bool ret = port.open(QextSerialPort::ReadWrite);
    if (ret)
        logger->info("Opened Serial Port!");
    else
        logger->error("Could not open serial port :(");
}

void Module_ScanningSonar::reset()
{
    QMessageBox::information(0, "Resetting sonar...", "Reset", QMessageBox::Ok);
    port.reset();
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

QByteArray Module_ScanningSonar::buildSwitchDataCommand()
{
    char range = settings.value("range", 50).toInt();
    char gain = settings.value("gain", 20).toInt();
    int trainAngle = settings.value("trainAngle", 70).toInt();
    char sectorWidth = settings.value("sectorWidth", 120).toInt();
    char stepSize = settings.value("stepSize", 1).toInt();
    char pulseLength = settings.value("pulseLength", 127).toInt();
    char dataPoints = settings.value("dataPoints", 25).toInt();
    char switchDelay = settings.value("switchDelay", 0).toInt();
    char frequency = settings.value("frequency", 0).toInt();

    QByteArray a;
    a.resize(27);
    a[0] = 0xFE;        // Switch data header
    a[1] = 0x44;        // Switch data header
    a[2] = 0x10;        // Head ID
    a[3] = range;       // Range
    a[4] = 0;           // Reserved
    a[5] = 0;           // Reserved
    a[6] = 0;           // Reserved
    a[7] = 0;           // Reserved
    a[8] = gain;        // Start Gain
    a[9] = 0;           // Reserved
    a[10]= 20;          // Absorption
    a[11]= trainAngle;  // Train Angle
    a[12]= sectorWidth; // Sector Width
    a[13]= stepSize;    // Step Size
    a[14]= pulseLength; // PulseLength
    a[15] = 0;          // Reserved
    a[16] = 0;          // Reserved
    a[17] = 0;          // Reserved
    a[18] = 0;          // Reserved
    a[19] = dataPoints; // DataPoints: 25 or 50
    a[20] = 0;          // Reserved
    a[21] = 0;          // Reserved
    a[22] = 0;          // Reserved
    a[23] = 0;          // Reserved
    a[24] = switchDelay;// Switch delay
    a[25] = frequency;  // Frequency
    a[26] = 0xFD;       // Termination Byte

    return a;
}
