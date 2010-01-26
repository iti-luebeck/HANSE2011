#include "module_uid.h"

#include <iostream>
#include <form_uid.h>
#include <qextserialenumerator.h>

Module_UID::Module_UID(QString moduleId)
    :RobotModule(moduleId)
{
    init();

    ScanForUIDs( settings.value("uidId", DEFAULT_UID_ID).toString() );
}

Module_UID::Module_UID(QString moduleId, QString deviceID)
    :RobotModule(moduleId)
{
    init();

    ScanForUIDs( deviceID );
}

void Module_UID::init()
{
    portSettings.BaudRate = BAUD115200;
    portSettings.DataBits = DATA_8;
    portSettings.Parity = PAR_NONE;
    portSettings.StopBits = STOP_1;
    portSettings.FlowControl = FLOW_OFF;
    portSettings.Timeout_Millisec = 200;
}

QextSerialPort* Module_UID::ScanForUIDs(QString Id) {

    UID_available = false;

    QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();

    const char sequence[] = {Module_UID::UID_IDENTIFY};
    char id[9];

    for (int i=0; i<ports.size(); i++) {
        logger->debug("port name: " + ports.at(i).portName);
        logger->debug("friendly name: " + ports.at(i).friendName);
        logger->debug("physical name: " + ports.at(i).physName);
        logger->debug("enumerator name: " + ports.at(i).enumName);
        logger->debug("===================================");
        port = new QextSerialPort(  "\\\\.\\"+ports.at(i).portName, portSettings);

        if ( !(port->open(QextSerialPort::ReadWrite) ) ) {
            logger->debug("Could not connect to " + port->errorString());
            port->close();
            continue;
        }

        logger->debug("connected successfully");

        port->flush();

        if ( (port->write(sequence, 1)) == -1) {
            logger->debug("Writing Opcode successfully");
            port->close();
            continue;
        }

        logger->debug("Could write to port");

        if ( (port->read(id, sizeof(id))) == -1 ) {
            logger->debug("No Id received");
            port->close();
            continue;
        }

        if (QString::fromAscii(id,sizeof(id)) == Id) {
            logger->info("Found UID with id " + QString::fromAscii(id,8) + " on " + ports.at(i).portName);
            UID_available = true;
            port->flush();
            return port;
        }
        else {
            logger->debug("Wrong Id; Received: " + QString::fromUtf8(id,8));
        }
        port->close();

    }
    return NULL;
}

bool Module_UID::UID_Available() {
    return UID_available;
}

unsigned char Module_UID::countBitsSet( unsigned char bitmask ) {
    unsigned char bits = 0;
    while (bitmask>0) {
        bits += (bitmask&1);
        bitmask = bitmask >> 1;
    }
    return bits;
}

bool Module_UID::SendCommand(unsigned char* sequence, unsigned char length, int msec) {
    if ( (port->write( (const char*)sequence, length )) == -1 ) return false;
    return true;
}

void Module_UID::ClosePort() {
    port->close();
}



QString Module_UID::UID_Identify() {
    QString identify;
    unsigned char sequence[] = {Module_UID::UID_IDENTIFY};
    char result[9];
    if ( !(SendCommand(sequence, sizeof(sequence), 0)) ) return NULL;
    qint64 ret = port->read(result,sizeof(result));
    if( ret == -1 ) return NULL;
    return QString::fromAscii(result, ret);
}

QString Module_UID::UID_Revision() {
    QString identify;
    unsigned char sequence[] = {Module_UID::UID_REVISION};
    char result[8];
    if ( !(SendCommand(sequence, sizeof(sequence), 0)) ) return NULL;
    //if (port->read(result,sizeof(result)) == -1) return NULL;
    qint64 ret = port->read(result,sizeof(result));
    if (ret==-1) return NULL;
    //return QString(result);
    return QString::fromAscii(result, ret);
}




bool Module_UID::I2C_EnterAckMode() {
    unsigned char sequence[] = {Module_UID::I2C_ENTERACKMODE};
    return SendCommand(sequence, sizeof(sequence), 0);
}

bool Module_UID::I2C_LeaveAckMode() {
    unsigned char sequence[] = {Module_UID::I2C_LEAVEACKMODE};
    return SendCommand(sequence, sizeof(sequence), 0);
}

bool Module_UID::I2C_Read(unsigned char address, short byteCount, unsigned char* result) {
    unsigned char sequence[] = {Module_UID::I2C_READ, address, byteCount};
    if (!SendCommand(sequence, sizeof(sequence) , 0)) return false;
    if ( (port->read((char*)result, byteCount)) == -1) return false;
    return true;
}

bool Module_UID::I2C_ReadRegisters(unsigned char address, unsigned char reg, short byteCount, unsigned char* result) {
    unsigned char sequence[] = {Module_UID::I2C_READREGISTER, address, reg, byteCount};
    if (!SendCommand(sequence, sizeof(sequence), 0)) return false;
    if ( (port->read((char*)result, byteCount)) == -1) return false;
    return true;
}

QVector<unsigned char> Module_UID::I2C_Scan() {
    unsigned char sequence[] = {Module_UID::I2C_SCAN};
    QVector<unsigned char> slaves(0);
    if (!SendCommand(sequence, sizeof(sequence), 0)) return slaves;
    unsigned char result[127];
    qint64 read = port->read((char*)result, 127);
    if (read != -1) {
        for (int i=0; i<read; i++) slaves.append(result[i]);
    }
    return slaves;
}

bool Module_UID::I2C_Speed(Module_UID::I2CSpeed speed) {
    unsigned char sequence[] = {Module_UID::I2C_SCAN, speed};
    return SendCommand(sequence, sizeof(sequence), 0);
}

bool Module_UID::I2C_TestSRF08Ready(unsigned char address) {
    return true;
}

bool Module_UID::I2C_Write(unsigned char address, unsigned char* data, short byteCount) {
    unsigned char sequence[] = {Module_UID::I2C_WRITE, address};
    if (!SendCommand(sequence, sizeof(sequence), 0)) return false;
    if (!SendCommand(data, byteCount, 0)) return false;
    return true;
}

bool Module_UID::I2C_WriteRegister(unsigned char address, unsigned char reg, unsigned char* data, short byteCount) {
    unsigned char sequence[] = {Module_UID::I2C_WRITE, address, reg};
    if (!SendCommand(sequence, sizeof(sequence), 0)) return false;
    if (!SendCommand(data, byteCount, 0)) return false;
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

bool Module_UID::I2C_DA_SendAck(unsigned char address) {
    unsigned char sequence[] = {Module_UID::I2C_DA_SENDACK};
    return SendCommand(sequence, sizeof(sequence), 0);
}

bool Module_UID::I2C_DA_SendByteAck(unsigned char address, unsigned char data) {
    unsigned char sequence[] = {Module_UID::I2C_DA_SENDBYTEACK};
    return SendCommand(sequence, sizeof(sequence), 0);
}

bool Module_UID::I2C_DA_SendByteNoAck(unsigned char address, unsigned char data) {
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
    unsigned char sequence[] = {Module_UID::ADC_ADC};
    unsigned char values[countBitsSet(bitmask)];
    if ( !(SendCommand(sequence, sizeof(sequence), 0)) ) return false;
    if (port->read((char*)result,sizeof(values)) == -1) return false;
    return true;
}

void Module_UID::reset()
{

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
    //
}
