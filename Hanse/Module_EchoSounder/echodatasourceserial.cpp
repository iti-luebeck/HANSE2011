#include "echodatasourceserial.h"
#include "echoreturndata.h"
#include "qextserialport.h"
#include "module_echosounder.h"
#include "echoswitchcommand.h"


EchoDataSourceSerial::EchoDataSourceSerial(Module_EchoSounder& parent)
    : EchoDataSource(parent)
{
    logger = Log4Qt::Logger::logger("EchoSerialReader");
    configurePort();
}

const EchoReturnData EchoDataSourceSerial::getNextPacket(){
    logger->debug("Sending switch data command.");

    EchoSwitchCommand cmd;
    cmd.range = parent.getSettingsValue("range").toInt();
    cmd.startGain = parent.getSettingsValue("gain").toInt();
    cmd.pulseLength = parent.getSettingsValue("pulseLength").toInt();
    cmd.profileMinRange = parent.getSettingsValue("profileMinRange").toInt();
    cmd.dataPoints = parent.getSettingsValue("dataPoints").toInt();
    cmd.profile = parent.getSettingsValue("profile").toInt();
    cmd.switchDelay = parent.getSettingsValue("switchDelay").toInt();
    cmd.frequency = parent.getSettingsValue("frequency").toInt();

    QByteArray sendArray = cmd.toSerialCmd();

    logger->trace("Sending: "+ QString(sendArray.toHex()));
    port->write(sendArray);

    QByteArray retData;

    int expectedLength;
    logger->debug("dataPoints:"+QString::number(parent.getSettingsValue("dataPoints").toInt()));
    if (parent.getSettingsValue("dataPoints").toInt()==50){
        expectedLength = 513;
    }else{
        expectedLength = 265;
    }
    int timeout = 500;
    while(timeout>0 && (retData.length()==0 || retData[retData.length()-1] != (char)0xFC)) {
        QByteArray ret = port->read(expectedLength - retData.length());
       // qDebug() << "ret" << ret ;
        retData.append(ret);
       // logger->error("retData"+QString::number(retData.size()));
        parent.msleep(5); timeout -= 5;
    }
    if (expectedLength - retData.length()>0) {
        logger->error("retData.length = "+QString::number(retData.length())+" - expectedLength = "+QString::number(expectedLength));
        logger->error("Received less than expected: "+QString::number(expectedLength - retData.length())+" bytes missing; expected="+QString::number(expectedLength));
    } else {
        logger->debug("received full packet");
    }

    logger->trace("Received in total: " + QString(retData.toHex()));

    EchoReturnData d(cmd,retData);

    return d;
}

void EchoDataSourceSerial::configurePort(){
    logger->info("Configuring serial port");
    PortSettings s;
    s.BaudRate = BAUD115200;
    s.DataBits = DATA_8;
    s.StopBits = STOP_1;
    s.Parity = PAR_NONE;
    s.Timeout_Millisec = 1;
    port = new QextSerialPort(parent.getSettingsValue("serialPort").toString(), s,QextSerialPort::Polling);
//  port = new QextSerialPort("COM6", s,QextSerialPort::Polling);
    bool ret = port->open(QextSerialPort::ReadWrite);
    if (ret){
        logger->debug("Opened serial port!");
    }else{
        parent.setHealthToSick("Could not open serial port '"+parent.getSettingsValue("serialPort").toString()+"'");
    }
}

bool EchoDataSourceSerial::isOpen()
{
    return port && port->isOpen();
}

EchoDataSourceSerial::~EchoDataSourceSerial()
{
}

void EchoDataSourceSerial::stop()
{
    logger->debug("Closing serial port.");
    if (port != NULL) {
        port->close();
        delete port;
        port = NULL;
    }
}

