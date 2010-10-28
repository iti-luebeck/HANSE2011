#include "sonardatasourceserial.h"
#include "sonarreturndata.h"
#include "qextserialport.h"
#include "module_scanningsonar.h"
#include "sonarswitchcommand.h"

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

    cmd.range = parent.getSettingsValue("range").toInt();
    cmd.startGain = parent.getSettingsValue("gain").toInt();
    cmd.trainAngle = parent.getSettingsValue("trainAngle").toInt();
    cmd.sectorWidth = parent.getSettingsValue("sectorWidth").toInt();
    cmd.stepSize = parent.getSettingsValue("stepSize").toInt();
    cmd.pulseLength = parent.getSettingsValue("pulseLength").toInt();
    cmd.dataPoints = parent.getSettingsValue("dataPoints").toInt();
    cmd.switchDelay = parent.getSettingsValue("switchDelay").toInt();
    cmd.frequency = parent.getSettingsValue("frequency").toInt();


    QByteArray sendArray = cmd.toSerialCmd();

    logger->trace("Sending: " + QString(sendArray.toHex()));
    port->write(sendArray);

    QByteArray retData;

    int expectedLength;
    logger->debug("dataPoints:"+QString::number(parent.getSettingsValue("dataPoints").toInt()));
    if (parent.getSettingsValue("dataPoints").toInt()==50)
        expectedLength = 513;
    else
        expectedLength = 265;

    int timeout = 100;
    while(timeout>0 && (retData.length()==0 || retData[retData.length()-1] != (char)0xFC)) {
        QByteArray ret = port->read(expectedLength - retData.length());
        retData.append(ret);
        parent.msleep(5); timeout -= 5;
    }
    if (expectedLength - retData.length()>0) {
        logger->error("Received less than expected: "+QString::number(expectedLength - retData.length())+" bytes missing; expected="+QString::number(expectedLength));
    } else {
        logger->debug("received full packet");
    }

    logger->trace("Received in total: " + QString(retData.toHex()));

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
    port = new QextSerialPort(parent.getSettingsValue("serialPort").toString(), s,QextSerialPort::Polling);
    bool ret = port->open(QextSerialPort::ReadWrite);
    if (ret)
        logger->debug("Opened serial port!");
    else
        parent.setHealthToSick("Could not open serial port '"+parent.getSettingsValue("serialPort").toString()+"'");
}

bool SonarDataSourceSerial::isOpen()
{
    return port && port->isOpen();
}

SonarDataSourceSerial::~SonarDataSourceSerial()
{
}

void SonarDataSourceSerial::stop()
{
    logger->debug("Closing serial port.");
    if (port != NULL) {
        port->close();
        delete port;
        port = NULL;
    }
}
