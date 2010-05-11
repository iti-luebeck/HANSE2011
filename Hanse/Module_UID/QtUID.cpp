#include "QtUID.h"

#include <iostream>
#include <qextserialport.h>
#include <qextserialenumerator.h>
#include <log4qt/logger.h>

#define DEBUG   1

UID::UID(QString Id)
{
    portSettings = new PortSettings();
    portSettings->BaudRate = BAUD115200;
    portSettings->DataBits = DATA_8;
    portSettings->Parity = PAR_NONE;
    portSettings->StopBits = STOP_1;
    portSettings->FlowControl = FLOW_OFF;
    portSettings->Timeout_Millisec = 100;

    l = Log4Qt::Logger::logger("QtUID");

    ScanForUIDs( Id );
}

void UID::MyThread::mySleep(long msec)
{
    msleep(msec);
}

QextSerialPort* UID::ScanForUIDs(QString IdBla) {

    QString Id = "UIDC0001";

    UID_available = false;

    QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();

    const char sequence[] = {UID::UID_IDENTIFY};
    char id[9];

    l->info("Trying out all available serial ports");

    // make sure that the pointer always points to something. otherwise
    // we will segfault on machines without any serial ports.
    port = new QextSerialPort(  "", *portSettings);

    for (int i=0; i<ports.size(); i++) {
        if (DEBUG) {
            l->debug("port name: "+ports.at(i).portName);
            l->debug("friendly name: "+ ports.at(i).friendName);
            l->debug("physical name: "+ ports.at(i).physName);
            l->debug("enumerator name: "+ ports.at(i).enumName);
            l->debug("===================================");
        }
 //       port = new QextSerialPort(  "\\\\.\\"+ports.at(i).portName, *portSettings);
        port = new QextSerialPort( ports.at(i).physName, *portSettings);

        //if ( !(port->open(QextSerialPort::ReadWrite) ) ) {
        if ( !(port->open(QIODevice::ReadWrite | QIODevice::Unbuffered) ) ) {
            l->debug("Could not connect: "+port->errorString());
            port->close();
            continue;
        }

        l->debug("connected successfully");

        port->flush();

        if ( (port->write(sequence, 1)) == -1) {
            l->debug("Writing Opcode failed");
            port->close();
            continue;
        }

        l->debug("Could write to port");
        MyThread::mySleep(1);
        int bytesRead = port->read(id, sizeof(id));
        if ( bytesRead < sizeof(id)) {
            l->debug("No Id received, read only "+QString::number(bytesRead)+ " bytes.");
            port->close();
            continue;
        }

        if (QString::fromAscii(id,sizeof(id)-1) == Id) {
            l->debug("Found UID with id " + QString::fromAscii(id,8) + " on " + ports.at(i).portName);
            UID_available = true;
            port->flush();
            return port;
        }
        else {
            l->debug("Received id: " + QString::fromUtf8(id,8)+"; expected: "+Id);
            l->debug("Wrong Id\r\nReceived: " + QString::fromUtf8(id,8));
        }
        port->close();

    }
    return NULL;
}

bool UID::UID_Available() {
    return UID_available;
}

unsigned char UID::countBitsSet( unsigned char bitmask ) {
    unsigned char bits = 0;
    while (bitmask>0) {
        bits += (bitmask&1);
        bitmask = bitmask >> 1;
    }
    return bits;
}

bool UID::SendCommand(unsigned char* sequence, unsigned char length, int msec) {
    if ( (port->write( (const char*)sequence, length )) == -1 ) return false;
    port->flush();
    return true;
}

void UID::ClosePort() {
    port->close();
}



QString UID::UID_Identify() {
    QString identify;
    unsigned char sequence[] = {UID::UID_IDENTIFY};
    char result[9];
    if ( !(SendCommand(sequence, sizeof(sequence), 0)) ) return NULL;
    qint64 ret = port->read(result,sizeof(result));
    if( ret == -1 ) return NULL;
    return QString::fromAscii(result, ret);
}

QString UID::UID_Revision() {
    QString identify;
    unsigned char sequence[] = {UID::UID_REVISION};
    char result[8];
    if ( !(SendCommand(sequence, sizeof(sequence), 0)) ) return NULL;
    //if (port->read(result,sizeof(result)) == -1) return NULL;
    qint64 ret = port->read(result,sizeof(result));
    if (ret==-1) return NULL;
    //return QString(result);
    return QString::fromAscii(result, ret);
}




bool UID::I2C_EnterAckMode() {
    unsigned char sequence[] = {UID::I2C_ENTERACKMODE};
    return SendCommand(sequence, sizeof(sequence), 0);
}

bool UID::I2C_LeaveAckMode() {
    unsigned char sequence[] = {UID::I2C_LEAVEACKMODE};
    return SendCommand(sequence, sizeof(sequence), 0);
}

bool UID::I2C_Read(unsigned char address, short byteCount, unsigned char* result) {
    unsigned char sequence[] = {UID::I2C_READ, address, byteCount};
    if (!SendCommand(sequence, sizeof(sequence) , 0)) return false;
    MyThread::mySleep(10);
    if ( (port->read((char*)result, byteCount)) == -1) return false;
    return true;
}

bool UID::I2C_ReadRegisters(unsigned char address, unsigned char reg, short byteCount, unsigned char* result) {
    unsigned char sequence[] = {UID::I2C_READREGISTER, address, reg, byteCount};
    if (!SendCommand(sequence, sizeof(sequence), 0)) return false;
    MyThread::mySleep(10);
    if ( (port->read((char*)result, byteCount)) != byteCount) return false;
    return true;
}

QVector<unsigned char> UID::I2C_Scan() {
    unsigned char sequence[] = {UID::I2C_SCAN};
    QVector<unsigned char> slaves(0);
    if (!SendCommand(sequence, sizeof(sequence), 0)) return slaves;
    unsigned char result[127];
    qint64 read = port->read((char*)result, 127);
    if (read != -1) {
        for (int i=0; i<read; i++) slaves.append(result[i]);
    }
    return slaves;
}

bool UID::I2C_Speed(UID::I2CSpeed speed) {
    unsigned char sequence[] = {UID::I2C_SCAN, speed};
    return SendCommand(sequence, sizeof(sequence), 0);
}

bool UID::I2C_TestSRF08Ready(unsigned char address) {
    unsigned char sequence[] = {UID::I2C_TESTSRF08READY, address};
    if (!SendCommand(sequence, sizeof(sequence), 0)) return false;
    char res[] = {0x00};
    if (port->read(res, 1) == -1) return false;
    if (res[0]!=0) return true;
    return false;
}

bool UID::I2C_Write(unsigned char address, unsigned char* data, short byteCount) {
    unsigned char sequence[] = {UID::I2C_WRITE, address, byteCount};
    if (!SendCommand(sequence, sizeof(sequence), 0)) return false;
    MyThread::mySleep(10);
    if (!SendCommand(data, byteCount, 0)) return false;
    MyThread::mySleep(1);
    return true;
}

bool UID::I2C_WriteRegister(unsigned char address, unsigned char reg, unsigned char* data, short byteCount) {
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

bool UID::I2C_DA_ReceiveAck() {
    unsigned char sequence[] = {UID::I2C_DA_RECEIVEACK};
    return SendCommand(sequence, sizeof(sequence), 0);
}

bool UID::I2C_DA_ReceiveByteAck() {
    unsigned char sequence[] = {UID::I2C_DA_RECEIVEBYTEACK};
    return SendCommand(sequence, sizeof(sequence), 0);
}

bool UID::I2C_DA_ReceiveByteNoAck() {
    unsigned char sequence[] = {UID::I2C_DA_RECEIVEBYTENOACK};
    return SendCommand(sequence, sizeof(sequence), 0);
}

bool UID::I2C_DA_RepStart() {
    unsigned char sequence[] = {UID::I2C_DA_REPSTART};
    return SendCommand(sequence, sizeof(sequence), 0);
}

bool UID::I2C_DA_SendAck() {
    unsigned char sequence[] = {UID::I2C_DA_SENDACK};
    return SendCommand(sequence, sizeof(sequence), 0);
}

bool UID::I2C_DA_SendByteAck(unsigned char data) {
    unsigned char sequence[] = {UID::I2C_DA_SENDBYTEACK};
    return SendCommand(sequence, sizeof(sequence), 0);
}

bool UID::I2C_DA_SendByteNoAck(unsigned char data) {
    unsigned char sequence[] = {UID::I2C_DA_RECEIVEBYTENOACK};
    return SendCommand(sequence, sizeof(sequence), 0);
}

bool UID::I2C_DA_Start() {
    unsigned char sequence[] = {UID::I2C_DA_START};
    return SendCommand(sequence, sizeof(sequence), 0);
}

bool UID::I2C_DA_Stop() {
    unsigned char sequence[] = {UID::I2C_DA_STOP};
    return SendCommand(sequence, sizeof(sequence), 0);
}

bool UID::UID_ADC(unsigned char bitmask, unsigned char* result) {
    unsigned char sequence[] = {UID::ADC_ADC, bitmask};
    unsigned char values = countBitsSet(bitmask);

    if ( !(SendCommand(sequence, sizeof(sequence), 0)) ) return false;

    qint64 res = port->read((char*)result,values);
    if (res==-1 || res<values) return false;

    return true;
}

bool UID::SPI_Speed(unsigned char speed) {
    unsigned char sequence[] = {UID::SPI_SPEED, speed};
    return SendCommand(sequence, sizeof(sequence), 0);
}

bool UID::SPI_SetPHA(bool phase) {
    unsigned char sequence[] = {UID::SPI_SETPHA, phase};
    return SendCommand(sequence, sizeof(sequence), 0);
}

bool UID::SPI_SetPOL(bool pol){
    unsigned char sequence[] = {UID::SPI_SETPOL, pol};
    return SendCommand(sequence, sizeof(sequence), 0);
}

bool UID::SPI_Read(unsigned char address, short byteCount, unsigned char* result) {
    unsigned char sequence[] = {UID::SPI_READ, address, byteCount};
    if (!SendCommand(sequence, sizeof(sequence) , 0)) return false;
    if ( (port->read((char*)result, byteCount)) == -1) return false;
    return true;
}

bool UID::SPI_Write(unsigned char address, unsigned char* data, short byteCount) {
    unsigned char sequence[] = {UID::SPI_WRITE, address, byteCount};
    if (!SendCommand(sequence, sizeof(sequence), 0)) return false;
    if (!SendCommand(data, byteCount, 0)) return false;
    return true;
}

bool UID::SPI_WriteRead(unsigned char address, unsigned char* data, short byteCount, unsigned char* result) {
    unsigned char sequence[] = {UID::SPI_WRITE, address, byteCount};
    if (!SendCommand(sequence, sizeof(sequence), 0)) return false;
    if (!SendCommand(data, byteCount, 0)) return false;
    if ( (port->read((char*)result, byteCount)) == -1) return false;
    return true;
}
