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
    : RobotModule(id)
{
    configurePort();

    //connect(&timer, SIGNAL(timeout()), this, SLOT(doNextScan()));
    //timer.start(settings.value("period", 1000).toInt());
}

Module_ScanningSonar::~Module_ScanningSonar()
{
    timer.stop();
    port.close();

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
    while(retData.length() < expectedLength) {
        QByteArray ret = port.read(expectedLength - retData.length());
        retData.append(ret);
        t.msleep(50);
        //logger->debug("Partial receive: " + QString(ret.toHex()));
    }
    logger->debug("Received in total: " + QString(retData.toHex()));

    // TODO: parse data

}

void Module_ScanningSonar::configurePort()
{
    port.setPortName(settings.value("port", "COM1").toString());
    port.setBaudRate(BAUD115200);
    port.setDataBits(DATA_8);
    port.setStopBits(STOP_1);
    port.setParity(PAR_NONE);
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
    char trainAngle = settings.value("trainAngle", 70).toInt();
    char sectorWidth = settings.value("sectorWidth", 120).toInt();
    char stepSize = settings.value("stepSize", 1).toInt();
    char pulseLength = settings.value("pulseLength", 127).toInt();
    char dataPoints = settings.value("dataPoints", 50).toInt();
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
