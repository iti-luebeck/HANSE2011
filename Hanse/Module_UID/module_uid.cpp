
#include <QtCore>
#include "module_uid.h"
#include "form_uid.h"
#include "QtUID.h"
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
    // TODO
}

void Module_UID::reset()
{
    if (uid != NULL) {
        uid->close();
        delete uid;
        uid = NULL;
    }
    uid = findUIDPort();
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
//       port = new QextSerialPort(  "\\\\.\\"+ports.at(i).portName, *portSettings);
    QextSerialPort* sport = new QextSerialPort( port->physName, *portSettings);

       const char sequence[] = {UID::UID_IDENTIFY};
       char id[9];

       //if ( !(port->open(QextSerialPort::ReadWrite) ) ) {
       if ( !(sport->open(QIODevice::ReadWrite | QIODevice::Unbuffered) ) ) {
           logger->debug("Could not connect: "+sport->errorString());
           sport->close();
           delete sport;
           return NULL;
       }

       logger->debug("connected successfully");

       sport->flush(); // ???

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

    //unsigned char sequence[4+byteCount];// = {UID::I2C_WRITEREGISTER, address, reg, (unsigned char)byteCount};
    unsigned char sequence[] = {UID::I2C_WRITEREGISTER, address, reg, (unsigned char)byteCount};
    /*
    int pos = 0;
    sequence[pos++] = UID::I2C_WRITEREGISTER;
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

    unsigned char sequence[] = {UID::I2C_READREGISTER, address, reg, byteCount};
    if (!SendCommand(sequence, sizeof(sequence), 0)) return false;
    if ( (uid->read((char*)result, byteCount)) != byteCount) return false;
    return true;
}


bool Module_UID::SendCommand(unsigned char* sequence, unsigned char length, int msec) {
    if (!uid) {
        logger->error("Can't do. UID not open.");
        return false;
    }

    if ( (uid->write( (const char*)sequence, length )) == -1 ) return false;
    uid->flush();
    return true;
}

bool Module_UID::I2C_Write(unsigned char address, unsigned char* data, short byteCount) {
    if (!uid) {
        logger->error("Can't do. UID not open.");
        return false;
    }

    unsigned char sequence[] = {UID::I2C_WRITE, address, byteCount};
    if (!SendCommand(sequence, sizeof(sequence), 0)) return false;
    if (!SendCommand(data, byteCount, 0)) return false;
    return true;
}

bool Module_UID::I2C_Read(unsigned char address, short byteCount, unsigned char* result) {
    if (!uid) {
        logger->error("Can't do. UID not open.");
        return false;
    }

    unsigned char sequence[] = {UID::I2C_READ, address, byteCount};
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
    unsigned char sequence[] = {UID::UID_IDENTIFY};
    char result[9];
    if ( !(SendCommand(sequence, sizeof(sequence), 0)) ) return NULL;
    qint64 ret = uid->read(result,sizeof(result));
    if( ret == -1 ) return NULL;
    return QString::fromAscii(result, ret);
}

QString Module_UID::UID_Revision() {
    if (!uid) {
        logger->error("Can't do. UID not open.");
        return "";
    }

    QString identify;
    unsigned char sequence[] = {UID::UID_REVISION};
    char result[8];
    if ( !(SendCommand(sequence, sizeof(sequence), 0)) ) return NULL;
    //if (port->read(result,sizeof(result)) == -1) return NULL;
    qint64 ret = uid->read(result,sizeof(result));
    if (ret==-1) return NULL;
    //return QString(result);
    return QString::fromAscii(result, ret);
}

bool Module_UID::UID_Available()
{
    return uid != NULL;
}

void Module_UID::doHealthCheck()
{
    QString Id = settings.value("uidId").toString();

    unsigned char sequence[] = {UID::UID_IDENTIFY};
    char id[9];

    if ( uid == NULL ) {
        return;
    }

    if (!SendCommand(sequence, 1, 50)) {
        setHealthToSick("Writing Opcode failed");
        return;
    }

    int bytesRead = uid->read(id, sizeof(id));
    if ( bytesRead < sizeof(id)) {
        setHealthToSick("No Id received, read only "+QString::number(bytesRead)+ " bytes.");
        return;
    }

    if (QString::fromAscii(id,sizeof(id)-1) != Id) {
        logger->debug("Received id: " + QString::fromUtf8(id,8)+"; expected: "+Id);
        setHealthToSick("Wrong Id, received: " + QString::fromUtf8(id,8));
        return;
    }

    setHealthToOk();
}
