#include "sonardatasourceserial.h"
#include "sonarreturndata.h"
#include "qextserialport.h"
#include "module_scanningsonar.h"
#include "sonarswitchcommand.h"

class SleeperThread : public QThread
{
public:
static void msleep(unsigned long msecs)
{
QThread::msleep(msecs);
}
};

SonarDataSourceSerial::SonarDataSourceSerial(Module_ScanningSonar& parent)
    : SonarDataSource(parent)
{
    logger = Log4Qt::Logger::logger("SonarSerialReader");
    configurePort();
}

const SonarReturnData SonarDataSourceSerial::getNextPacket()
{
    logger->debug("Sending switch data command.");

    SonarSwitchCommand cmd;

    cmd.range = parent.getSettings().value("range").toInt();
    cmd.startGain = parent.getSettings().value("gain").toInt();
    cmd.trainAngle = parent.getSettings().value("trainAngle").toInt();
    cmd.sectorWidth = parent.getSettings().value("sectorWidth").toInt();
    cmd.stepSize = parent.getSettings().value("stepSize").toInt();
    cmd.pulseLength = parent.getSettings().value("pulseLength").toInt();
    cmd.dataPoints = parent.getSettings().value("dataPoints").toInt();
    cmd.switchDelay = parent.getSettings().value("switchDelay").toInt();
    cmd.frequency = parent.getSettings().value("frequency").toInt();


    QByteArray sendArray = cmd.toSerialCmd();

    logger->trace("Sending: " + QString(sendArray.toHex()));
    port->write(sendArray);

    QByteArray retData;
    SleeperThread t;

    int expectedLength;
    if (parent.getSettings().value("dataPoints", 50).toInt())
        expectedLength = 513;
    else
        expectedLength = 265;

    int timeout = 1000;
    while(timeout>0 && (retData.length()==0 || retData[retData.length()-1] != (char)0xFC)) {
        QByteArray ret = port->read(expectedLength - retData.length());
        retData.append(ret);
        t.msleep(5); timeout -= 5;
    }
    if (expectedLength - retData.length()>0) {
        logger->error("Received less than expected: "+QString::number(expectedLength - retData.length())+" bytes missing.");
    }
    //logger->trace("Received in total: " + QString(retData.toHex()));

    SonarReturnData d(cmd,retData);

    return d;
}

void SonarDataSourceSerial::configurePort()
{
    logger->info("Configuring serial port");
    PortSettings s;
    s.BaudRate = BAUD115200;
    s.DataBits = DATA_8;
    s.StopBits = STOP_1;
    s.Parity = PAR_NONE;
    s.Timeout_Millisec = 1;
    port = new QextSerialPort(parent.getSettings().value("serialPort").toString(), s,QextSerialPort::Polling);
    bool ret = port->open(QextSerialPort::ReadWrite);
    if (ret)
        logger->debug("Opened serial port!");
    else
        parent.setHealthToSick("Could not open serial port '"+parent.getSettings().value("serialPort").toString()+"'");
}

bool SonarDataSourceSerial::isOpen()
{
    return port && port->isOpen();
}
