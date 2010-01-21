#ifndef MODULE_UID_H
#define MODULE_UID_H

#include "robotmodule.h"
#include "Module_UID_global.h"
#include <string>
#include <QtCore/QObject>
#include <qextserialport.h>
#include <QVector>
#include <QString>
#include <QtGlobal>

class MODULE_UIDSHARED_EXPORT Module_UID : public RobotModule {
    Q_OBJECT
public:

    enum I2CSpeed
    {
        kHz100 = 0xCC,
        kHz200 = 0xF0,
        khz400 = 0xFF
    };

    Module_UID(QString moduleId, QString deviceId);
    bool UID_Available();

    QString UID_Identify();
    QString UID_Revision();

    bool I2C_EnterAckMode();
    bool I2C_LeaveAckMode();
    bool I2C_Read(unsigned char address, short byteCount, unsigned char* result);
    bool I2C_ReadRegisters(unsigned char address, unsigned char reg, short byteCount, unsigned char* result);
    QVector<unsigned char> I2C_Scan();
    bool I2C_Speed(I2CSpeed speed);
    bool I2C_TestSRF08Ready(unsigned char address);
    bool I2C_Write(unsigned char address, unsigned char* data, short byteCount);
    bool I2C_WriteRegister(unsigned char address, unsigned char reg, unsigned char* data, short byteCount);
    bool I2C_DA_ReceiveAck();
    bool I2C_DA_ReceiveByteAck();
    bool I2C_DA_ReceiveByteNoAck();
    bool I2C_DA_RepStart();
    bool I2C_DA_SendAck(unsigned char address);
    bool I2C_DA_SendByteAck(unsigned char address, unsigned char data);
    bool I2C_DA_SendByteNoAck(unsigned char address, unsigned char data);
    bool I2C_DA_Start();
    bool I2C_DA_Stop();
    bool UID_ADC(unsigned char bitmask, unsigned char* result);

    // inherited from RobotModule
    QWidget* createView(QWidget* parent);
    // inherited from RobotModule
    QList<RobotModule*> getDependencies();

public slots:
    // inherited from RobotModule
    void reset();
    // inherited from RobotModule
    void terminate();
private:

    enum Opcode
    {
        UID_IDENTIFY = 0x05,
        UID_REVISION = 0x0C,
        I2C_ENTERACKMODE = 0x03,
        I2C_LEAVEACKMODE = 0x06,
        I2C_READ = 0x09,
        I2C_READREGISTER = 0x0A,
        I2C_SCAN = 0x0F,
        I2C_SPEED = 0x11,
        I2C_TESTSRF08READY = 0x2E,
        I2C_WRITE = 0x18,
        I2C_WRITEREGISTER = 0x1B,
        I2C_DA_RECEIVEACK = 0x1D,
        I2C_DA_RECEIVEBYTEACK = 0x1E,
        I2C_DA_RECEIVEBYTENOACK = 0x21,
        I2C_DA_REPSTART = 0x22,
        I2C_DA_SENDACK = 0x24,
        I2C_DA_SENDBYTEACK = 0x27,
        I2C_DA_SENDBYTENOACK = 0x28,
        I2C_DA_START = 0x2B,
        I2C_DA_STOP = 0x2D,
        SPI_READ = 0x12,
        SPI_SPEED = 0x14,
        SPI_WRITE = 0x17,
        ADC_ADC = 0x00
    };

    QextSerialPort* port;
    PortSettings portSettings;

    QextSerialPort* ScanForUIDs( QString Id );
    void ClosePort();
    QString IdentifyCommand(unsigned char* sequence);
    bool SendCommand(unsigned char* sequence, unsigned char length, int msec);
    bool WaitForSerialData( short amount );
    unsigned char countBitsSet( unsigned char bitmask );

    bool UID_available;
};

#endif // MODULE_UID_H
