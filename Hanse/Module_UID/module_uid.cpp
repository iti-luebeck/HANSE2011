
#include <QtCore>
#include "module_uid.h"
#include "form_uid.h"
#include <qextserialport.h>
#include <qextserialenumerator.h>

Module_UID::Module_UID(QString moduleId)
    :RobotModule(moduleId)
{

    portSettings = new PortSettings();
    portSettings->BaudRate = BAUD115200;
    portSettings->DataBits = DATA_8;
    portSettings->Parity = PAR_NONE;
    portSettings->StopBits = STOP_1;
    portSettings->FlowControl = FLOW_OFF;
    portSettings->Timeout_Millisec = 500;

    setDefaultValue("uidId", "UIDC0001");

    uid = NULL;
    reset();
}

Module_UID::~Module_UID()
{
    if (uid != NULL) {
        uid->close();
        delete uid;
        uid = NULL;
    }
}

void Module_UID::reset()
{
    RobotModule::reset();

    if (!getSettings().value("enabled").toBool())
        return;

    if (uid != NULL) {
        uid->close();
        delete uid;
        uid = NULL;

    }
    uid = findUIDPort();
    data["revision"] = this->UID_Revision();
    data["identity"] = this->UID_Identify();
}

QList<RobotModule*> Module_UID::getDependencies()
{
    return QList<RobotModule*>();
}

QWidget* Module_UID::createView(QWidget* parent)
{
    return new FormUID(this, parent);
}

void Module_UID::terminate()
{
    RobotModule::terminate();
}

QextSerialPort* Module_UID::findUIDPort()
{
    QString Id = settings.value("uidId").toString();

    QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();

    logger->info("Trying out all available serial ports");

    QextSerialPort *p = NULL;

    foreach (QextPortInfo port, ports) {
            logger->debug("port name: "+port.portName);
            logger->debug("friendly name: "+ port.friendName);
            logger->debug("physical name: "+ port.physName);
            logger->debug("enumerator name: "+ port.enumName);
            logger->debug("===================================");

            p = tryOpenPort(Id, &port);

            if (p != NULL)
                break;
    }

    if (!p) {
        setHealthToSick("Didn't find any UID.");
    } else {
        setHealthToOk();
    }

    return p;
}

QextSerialPort* Module_UID::tryOpenPort(QString Id, QextPortInfo *port)
{
    //QextSerialPort* sport = new QextSerialPort( port->physName, *portSettings, QextSerialPort::Polling);
    QextSerialPort* sport = new QextSerialPort( port->portName, *portSettings, QextSerialPort::Polling);

    const char sequence[] = {Module_UID::UID_IDENTIFY};
    char id[9];

    //if ( !(port->open(QextSerialPort::ReadWrite) ) ) {
    if ( !(sport->open(QIODevice::ReadWrite | QIODevice::Unbuffered) ) ) {
        logger->debug("Could not open: "+sport->errorString());
        sport->close();
        delete sport;
        return NULL;
    }

    logger->debug("connected successfully");

    sport->flush();
    if (sport->bytesAvailable()>0) {
        logger->error("found some bytes in the uid serial buffer. discard them.");
        sport->flush();
        sport->readAll(); // don't care for the data
    }

    if ( (sport->write(sequence, 1)) == -1) {
        logger->debug("Writing Opcode failed");
        sport->close();
        delete sport;
        return NULL;
    }

    logger->debug("Could write to port");
    int bytesRead = sport->read(id, sizeof(id));
    if ( bytesRead < sizeof(id)) {
        logger->debug("No Id received, read only "+QString::number(bytesRead)+ " bytes.");
        sport->close();
        delete sport;
        return NULL;
    }

    if (QString::fromAscii(id,sizeof(id)-1) != Id) {
        logger->debug("Received id: " + QString::fromUtf8(id,8)+"; expected: "+Id);
        logger->debug("Wrong Id\r\nReceived: " + QString::fromUtf8(id,8));
        sport->close();
        delete sport;
        return NULL;
    }

    logger->debug("Found UID with id " + QString::fromAscii(id,8) + " on " + port->portName);
    sport->flush();
    return sport;

}

bool Module_UID::I2C_WriteRegister(unsigned char address, unsigned char reg, unsigned char* data, short byteCount) {
    if (!uid) {
        logger->error("Can't do. UID not open.");
        return false;
    }

    //unsigned char sequence[4+byteCount];// = {Module_UID::I2C_WRITEREGISTER, address, reg, (unsigned char)byteCount};
    unsigned char sequence[] = {Module_UID::I2C_WRITEREGISTER, address, reg, (unsigned char)byteCount};
    /*
    int pos = 0;
    sequence[pos++] = Module_UID::I2C_WRITEREGISTER;
    sequence[pos++] = address;
    sequence[pos++] = reg;
    sequence[pos++] = (unsigned char)byteCount;

    for (int i=0; i<byteCount; i++)
        sequence[pos++]=data[i];
        */
    if (!SendCommand(sequence, sizeof(sequence), 0)) return false;
    if (!SendCommand(data, byteCount, 0)) return false;
    return true;
}

bool Module_UID::I2C_ReadRegisters(unsigned char address, unsigned char reg, short byteCount, unsigned char* result) {
    if (!uid) {
        logger->error("Can't do. UID not open.");
        return false;
    }

    unsigned char sequence[] = {Module_UID::I2C_READREGISTER, address, reg, byteCount};
    if (!SendCommand(sequence, sizeof(sequence), 0)) return false;
    if ( (uid->read((char*)result, byteCount)) != byteCount) return false;
    return true;
}


bool Module_UID::SendCommand(unsigned char* sequence, unsigned char length, int msec) {
    if (!uid) {
        logger->error("Can't do. UID not open.");
        return false;
    }

    if ( (uid->write( (const char*)sequence, length )) == -1 ) {
        // this means almost certainly that the serial connection to the UID has died.
        setHealthToSick(uid->portName()+": "+uid->errorString());
        return false;
    }
    uid->flush();
    return true;
}

bool Module_UID::I2C_Write(unsigned char address, unsigned char* data, short byteCount) {
    if (!uid) {
        logger->error("Can't do. UID not open.");
        return false;
    }

    unsigned char sequence[] = {Module_UID::I2C_WRITE, address, byteCount};
    if (!SendCommand(sequence, sizeof(sequence), 0)) return false;
    if (!SendCommand(data, byteCount, 0)) return false;
    return true;
}

bool Module_UID::I2C_Read(unsigned char address, short byteCount, unsigned char* result) {
    if (!uid) {
        logger->error("Can't do. UID not open.");
        return false;
    }

    unsigned char sequence[] = {Module_UID::I2C_READ, address, byteCount};
    if (!SendCommand(sequence, sizeof(sequence) , 0)) return false;
    if ( (uid->read((char*)result, byteCount)) != byteCount) return false;
    return true;
}

QString Module_UID::UID_Identify() {
    if (!uid) {
        logger->error("Can't do. UID not open.");
        return "";
    }

    QString identify;
    unsigned char sequence[] = {Module_UID::UID_IDENTIFY};
    char result[9];
    if ( !(SendCommand(sequence, sizeof(sequence), 0)) ) {
        return "";
    }
    qint64 ret = uid->read(result,sizeof(result));
    if( ret == -1 ) {
        setHealthToSick("serial error: "+uid->errorString());
        return "";
    }
    if (ret < 9) {
        setHealthToSick("Short read.");
        return "";
    }
    return QString::fromAscii(result, ret-1);
}

QString Module_UID::UID_Revision() {
    if (!uid) {
        logger->error("Can't do. UID not open.");
        return "";
    }

    QString identify;
    unsigned char sequence[] = {Module_UID::UID_REVISION};
    char result[8];
    if ( !(SendCommand(sequence, sizeof(sequence), 0)) ) return NULL;
    //if (port->read(result,sizeof(result)) == -1) return NULL;
    qint64 ret = uid->read(result,sizeof(result));
    if (ret==-1) return NULL;
    //return QString(result);
    return QString::fromAscii(result, ret-1);
}

bool Module_UID::UID_Available()
{
    return uid != NULL;
}

void Module_UID::doHealthCheck()
{
    if (!getSettings().value("enabled").toBool())
        return;

    QString Id = settings.value("uidId").toString();

    if ( uid == NULL ) {
        setHealthToSick("No uid open.");
        return;
    }

    QString received = UID_Identify();
    if ( received.length()==0) {
        return;
    }
    if ( received.length()>0 && received != Id) {
        setHealthToSick("Wrong Id, received: " + received);
        return;
    }

    setHealthToOk();
}


QVector<unsigned char> Module_UID::I2C_Scan() {
    unsigned char sequence[] = {Module_UID::I2C_SCAN};
    QVector<unsigned char> slaves(0);
    if (!SendCommand(sequence, sizeof(sequence), 0)) return slaves;
    unsigned char result[127];
    qint64 read = uid->read((char*)result, 127);
    if (read != -1) {
        for (int i=0; i<read; i++) slaves.append(result[i]);
    }
    return slaves;
}

bool Module_UID::SPI_Speed(unsigned char speed) {
    unsigned char sequence[] = {Module_UID::SPI_SPEED, speed};
    return SendCommand(sequence, sizeof(sequence), 0);
}

bool Module_UID::SPI_SetPHA(bool phase) {
    unsigned char sequence[] = {Module_UID::SPI_SETPHA, phase};
    return SendCommand(sequence, sizeof(sequence), 0);
}

bool Module_UID::SPI_SetPOL(bool pol){
    unsigned char sequence[] = {Module_UID::SPI_SETPOL, pol};
    return SendCommand(sequence, sizeof(sequence), 0);
}

bool Module_UID::SPI_Read(unsigned char address, short byteCount, unsigned char* result) {
    unsigned char sequence[] = {Module_UID::SPI_READ, address, byteCount};
    if (!SendCommand(sequence, sizeof(sequence) , 0)) return false;
    if ( (uid->read((char*)result, byteCount)) == -1) return false;
    return true;
}

bool Module_UID::SPI_Write(unsigned char address, unsigned char* data, short byteCount) {
    unsigned char sequence[] = {Module_UID::SPI_WRITE, address, byteCount};
    if (!SendCommand(sequence, sizeof(sequence), 0)) return false;
    if (!SendCommand(data, byteCount, 0)) return false;
    return true;
}

bool Module_UID::SPI_WriteRead(unsigned char address, unsigned char* data, short byteCount, unsigned char* result) {
    unsigned char sequence[] = {Module_UID::SPI_WRITE, address, byteCount};
    if (!SendCommand(sequence, sizeof(sequence), 0)) return false;
    if (!SendCommand(data, byteCount, 0)) return false;
    if ( (uid->read((char*)result, byteCount)) == -1) return false;
    return true;
}

bool Module_UID::I2C_DA_ReceiveAck() {
    unsigned char sequence[] = {Module_UID::I2C_DA_RECEIVEACK};
    return SendCommand(sequence, sizeof(sequence), 0);
}

bool Module_UID::I2C_DA_ReceiveByteAck() {
    unsigned char sequence[] = {Module_UID::I2C_DA_RECEIVEBYTEACK};
    return SendCommand(sequence, sizeof(sequence), 0);
}

bool Module_UID::I2C_DA_ReceiveByteNoAck() {
    unsigned char sequence[] = {Module_UID::I2C_DA_RECEIVEBYTENOACK};
    return SendCommand(sequence, sizeof(sequence), 0);
}

bool Module_UID::I2C_DA_RepStart() {
    unsigned char sequence[] = {Module_UID::I2C_DA_REPSTART};
    return SendCommand(sequence, sizeof(sequence), 0);
}

bool Module_UID::I2C_DA_SendAck() {
    unsigned char sequence[] = {Module_UID::I2C_DA_SENDACK};
    return SendCommand(sequence, sizeof(sequence), 0);
}

bool Module_UID::I2C_DA_SendByteAck(unsigned char data) {
    unsigned char sequence[] = {Module_UID::I2C_DA_SENDBYTEACK};
    return SendCommand(sequence, sizeof(sequence), 0);
}

bool Module_UID::I2C_DA_SendByteNoAck(unsigned char data) {
    unsigned char sequence[] = {Module_UID::I2C_DA_RECEIVEBYTENOACK};
    return SendCommand(sequence, sizeof(sequence), 0);
}

bool Module_UID::I2C_DA_Start() {
    unsigned char sequence[] = {Module_UID::I2C_DA_START};
    return SendCommand(sequence, sizeof(sequence), 0);
}

bool Module_UID::I2C_DA_Stop() {
    unsigned char sequence[] = {Module_UID::I2C_DA_STOP};
    return SendCommand(sequence, sizeof(sequence), 0);
}

bool Module_UID::UID_ADC(unsigned char bitmask, unsigned char* result) {
    unsigned char sequence[] = {Module_UID::ADC_ADC, bitmask};
    unsigned char values = countBitsSet(bitmask);

    if ( !(SendCommand(sequence, sizeof(sequence), 0)) ) return false;

    qint64 res = uid->read((char*)result,values);
    if (res==-1 || res<values) return false;

    return true;
}

bool Module_UID::I2C_Speed(Module_UID::I2CSpeed speed) {
    unsigned char sequence[] = {Module_UID::I2C_SCAN, speed};
    return SendCommand(sequence, sizeof(sequence), 0);
}


bool Module_UID::I2C_EnterAckMode() {
    unsigned char sequence[] = {Module_UID::I2C_ENTERACKMODE};
    return SendCommand(sequence, sizeof(sequence), 0);
}

unsigned char Module_UID::countBitsSet( unsigned char bitmask ) {
    unsigned char bits = 0;
    while (bitmask>0) {
        bits += (bitmask&1);
        bitmask = bitmask >> 1;
    }
    return bits;
}
