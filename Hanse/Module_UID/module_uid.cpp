
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
#ifdef OS_UNIX // linux only supports timeouts in tenth of seconds and rounds down
    portSettings->Timeout_Millisec = 100;
#else
    portSettings->Timeout_Millisec = 10;
#endif

    setDefaultValue("uidId", "UIDC0001");

    lastError = 0;

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

#ifdef OS_UNIX // this platform-independent library puts the port name in different fields depending on the platform
    QextSerialPort* sport = new QextSerialPort( port->physName, *portSettings, QextSerialPort::Polling);
#else
    QextSerialPort* sport = new QextSerialPort( port->portName, *portSettings, QextSerialPort::Polling);
#endif

    const char sequence[] = {Module_UID::UID_IDENTIFY};
    char id[9];

    logger->debug("Using timeout of "+QString::number(portSettings->Timeout_Millisec)+" ms.");
    
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

    logger->trace("I2C_WriteRegister");

    if (!uid) {
        logger->error("Can't do. UID not open.");
        return false;
    }

    QByteArray send;
    send += Module_UID::I2C_WRITEREGISTER;
    send += address;
    send += reg;
    send += byteCount;
    send.append((const char*)data, byteCount);

    return SendCheckCommand(send);

}

bool Module_UID::I2C_ReadRegisters(unsigned char address, unsigned char reg, short byteCount, unsigned char* result) {

    logger->trace("I2C_ReadRegisters");

    QByteArray send;
    send += Module_UID::I2C_READREGISTER;
    send += address;
    send += reg;
    send += byteCount;

    return SendCheckCommand(send,(char*)result, byteCount);
}


bool Module_UID::SendCommand(unsigned char* sequence, unsigned char length, int msec) {
    logger->trace("SendCommand");

    if (!uid) {
        logger->error("Can't do. UID not open.");
        return false;
    }

    uid->flush();
    if (uid->bytesAvailable()>0) {
        logger->error("found some bytes in the uid serial buffer. discard them.");
        uid->flush();
        uid->readAll(); // don't care for the data
    }

    if ( (uid->write( (const char*)sequence, length )) == -1 ) {
        // this almost certainly means that the serial connection to the UID has died.
        setHealthToSick(uid->portName()+": "+uid->errorString());
        return false;
    }
    uid->flush();
    return true;
}

bool Module_UID::SendCheckCommand(QByteArray &send, char* recv, int recv_length)
{
    logger->trace("SendCheckCommand");

    if (!SendCommand2(send,recv, recv_length)) {
        return false;
    }

    if (!CheckErrorcode()) {
        return false;
    }

    return true;

}

bool Module_UID::CheckErrorcode()
{
    logger->trace("CheckErrorcode");

    char r[1];
    r[0] = 0;
    QByteArray s;
    s += 0xFF;
    if (!SendCommand2(s,r,1)) {
        setHealthToSick("Didn't get an error code!");
        return false;
    }
    lastError = r[0];
    logger->trace("Got error code: "+QString::number(lastError));
    return lastError == 0;
}

bool Module_UID::SendCommand2(QByteArray& send, char* recv, int recv_length) {
    logger->trace("SendCommand2");

    logger->trace("Sending: 0x"+QString::fromAscii(send.toHex()));

    if (!uid) {
        logger->error("Can't do. UID not open.");
        lastError = E_USB;
        return false;
    }

    uid->flush();
    if (uid->bytesAvailable()>0) {
        logger->error("found some bytes in the uid serial buffer. discarding them.");
        uid->flush();
        uid->readAll(); // don't care for the data
    }

    int sendRet = uid->write(send);
    if (sendRet==-1) {
        setHealthToSick("Connection to UID died: "+uid->errorString());
        lastError = E_USB;
        return false;
    }

    if (sendRet<send.size()) {
        setHealthToSick("Could not write the whole buffer.");
        lastError = E_USB;
        return false;
    }
    uid->flush();

    // we expect a return
    if (recv_length) {
        int recvRet = uid->read(recv,recv_length);
        if (recvRet==-1) {
            setHealthToSick("Connection to UID died: "+uid->errorString());
            lastError = E_USB;
            return false;
        }

//        if (recvRet<recv_length) {
//            setHealthToSick("Received less data from UID than expected: "+QString::number(recv_length-recvRet)+" bytes missing.");
//            lastError = E_SHORT_READ;
//            // this is an indication for an error. but wait for check command
//            return true;
//        }
    }

    if (uid->bytesAvailable()>0) {
        setHealthToSick("found some orphaned bytes in the uid serial buffer after doing a command 0x"+QString::number(send[0],16));
        uid->flush();
        uid->readAll(); // don't care for the data
    }

    lastError = E_NO_ERROR;
    return true;
}

bool Module_UID::I2C_Write(unsigned char address, unsigned char* data, short byteCount) {
    logger->trace("I2C_Write");

    QByteArray send;
    send += Module_UID::I2C_WRITE;
    send += address;
    send += byteCount;
    send.append((char*)data, byteCount);
    return SendCheckCommand(send);
}

bool Module_UID::I2C_Read(unsigned char address, short byteCount, unsigned char* result) {
    logger->trace("I2C_Read");

    QByteArray send;
    send += Module_UID::I2C_READ;
    send += address;
    send += byteCount;
    return SendCheckCommand(send,(char*)result,byteCount);
}

QString Module_UID::UID_Identify() {
    logger->trace("UID_Identify");

    if (!uid) {
        logger->error("Can't do. UID not open.");
        return "";
    }

    char result[9];
    result[9]=0;
    QByteArray send;
    send += Module_UID::UID_IDENTIFY;
    bool ret = SendCheckCommand(send,(char*)result,9);
    if (!ret) {
        setHealthToSick(getLastError());
        return "";
    }

    return QString::fromAscii(result);

}

QString Module_UID::UID_Revision() {
    logger->trace("UID_Revision");

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
    logger->trace("doHealthCheck");
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
    logger->trace("I2C_Scan");
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
    QByteArray send;
    send += Module_UID::SPI_SPEED;
    send += speed;
    return SendCheckCommand(send);
}

bool Module_UID::SPI_SetPHA(bool phase) {
    QByteArray send;
    send += Module_UID::SPI_SETPHA;
    send += phase;
    return SendCheckCommand(send);
}

bool Module_UID::SPI_SetPOL(bool pol){
    QByteArray send;
    send += Module_UID::SPI_SETPOL;
    send += pol;
    return SendCheckCommand(send);
}

bool Module_UID::SPI_Read(unsigned char address, short byteCount, unsigned char* result) {
    QByteArray send;
    send += Module_UID::SPI_READ;
    send += address;
    send += byteCount;
    return SendCheckCommand(send, (char*)result, byteCount);
}

bool Module_UID::SPI_Write(unsigned char address, unsigned char* data, short byteCount) {
    QByteArray send;
    send += Module_UID::SPI_WRITE;
    send += address;
    send += byteCount;
    send.append((char*)data, byteCount);
    return SendCheckCommand(send);
}

bool Module_UID::SPI_WriteRead(unsigned char address, unsigned char* data, short byteCount, unsigned char* result) {
    QByteArray send;
    send += Module_UID::SPI_READWRITE;
    send += address;
    send += byteCount;
    send.append((char*)data, byteCount);
    return SendCheckCommand(send, (char*)result, byteCount);
}

bool Module_UID::I2C_Speed(Module_UID::I2CSpeed speed) {
    QByteArray send;
    send += Module_UID::I2C_SPEED;
    send += speed;
    return SendCheckCommand(send);
}

unsigned char Module_UID::countBitsSet( unsigned char bitmask ) {
    unsigned char bits = 0;
    while (bitmask>0) {
        bits += (bitmask&1);
        bitmask = bitmask >> 1;
    }
    return bits;
}

QString Module_UID::getLastError()
{
    switch(lastError)
    {
    case E_NO_ERROR: return "No error";
    case E_I2C_LOW: return "I2C Bus is low.";
    case E_I2C_START: return "Could not send I2C START";
    case E_I2C_MT_SLA_ACK: return "Slave didn't respond to SLA+W";
    case E_I2C_MR_SLA_ACK: return "Slave didn't respond to SLA+R";
    case E_I2C_MT_DATA_ACK: return "Slave didn't ACK data of SLA+W";
    case E_I2C_MR_DATA_ACK: return "Slave didn't ACK data of SLA+R";
    case E_I2C_MR_DATA_NACK: return "Slave didn't NACK data of SLA+R";
    case E_SHORT_READ: return "Read less bytes than expected.";
    case E_EXTRA_READ: return "Read more bytes than expected.";
    case E_USB: return "Serial connection to UID broke down.";
    default: return QString("Unknown error: %1").arg(lastError);
    }
}

bool Module_UID::isSlaveProblem()
{
    switch (lastError) {
    case E_I2C_MT_SLA_ACK:
    case E_I2C_MR_SLA_ACK:
    case E_I2C_MT_DATA_ACK:
    case E_I2C_MR_DATA_ACK:
    case E_I2C_MR_DATA_NACK:
    case E_EXTRA_READ:
        return true;
    default:
        return false;
    }
}
