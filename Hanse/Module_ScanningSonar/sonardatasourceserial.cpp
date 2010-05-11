#include "sonardatasourceserial.h"
#include "sonarreturndata.h"
#include "qextserialport.h"
#include "module_scanningsonar.h"

class SleeperThread : public QThread
{
public:
static void msleep(unsigned long msecs)
{
QThread::msleep(msecs);
}
};

SonarDataSourceSerial::SonarDataSourceSerial(Module_ScanningSonar& parent, QString port)
    : SonarDataSource(parent)
{
    logger = Log4Qt::Logger::logger("SonarSerialReader");
    configurePort();
}

SonarReturnData* SonarDataSourceSerial::getNextPacket()
{
    logger->debug("Sending switch data command.");
    QByteArray sendArray = buildSwitchDataCommand();
    logger->debug("Sending: " + QString(sendArray.toHex()));
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
    logger->debug("Received in total: " + QString(retData.toHex()));

    SonarReturnData* d = new SonarReturnData(retData);

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
    s.FlowControl = FLOW_OFF;
    s.Timeout_Millisec = 1;
    port = new QextSerialPort(parent.getSettings().value("serialPort").toString(), s);
    bool ret = port->open(QextSerialPort::ReadWrite);
    if (ret)
        logger->info("Opened Serial Port!");
    else
        logger->error("Could not open serial port :(");
}

QByteArray SonarDataSourceSerial::buildSwitchDataCommand()
{
    char range = parent.getSettings().value("range").toInt();
    char gain = parent.getSettings().value("gain").toInt();
    int trainAngle = parent.getSettings().value("trainAngle").toInt();
    char sectorWidth = parent.getSettings().value("sectorWidth").toInt();
    char stepSize = parent.getSettings().value("stepSize").toInt();
    char pulseLength = parent.getSettings().value("pulseLength").toInt();
    char dataPoints = parent.getSettings().value("dataPoints").toInt();
    char switchDelay = parent.getSettings().value("switchDelay").toInt();
    char frequency = parent.getSettings().value("frequency").toInt();

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
