#include "module_compass.h"
#include "compass_form.h"
#include <Module_UID/module_uid.h>

#define COMPASS_CMD_ACCEL_DATA		0x40
#define COMPASS_CMD_MAG_DATA		0x45
#define COMPASS_CMD_HEADING_DATA	0x50
#define COMPASS_CMD_TILT_DATA		0x55

#define COMPASS_CMD_GET_OPMODE		0x65
#define COMPASS_CMD_ORIENT_LEVEL	0x72
#define COMPASS_CMD_ORIENT_SIDEWAYS	0x73
#define COMPASS_CMD_ORIENT_FLAT		0x74

#define COMPASS_CMD_RUN_ENTER		0x75
#define COMPASS_CMD_STANDBY_ENTER	0x76

#define COMPASS_CMD_CALIB_START		0x71
#define COMPASS_CMD_CALIB_STOP		0x7E

#define COMPASS_CMD_SOFT_RESET		0x82
#define COMPASS_CMD_SLEEP_ENTER		0x83
#define COMPASS_CMD_SLEEP_LEAVE		0x84

#define COMPASS_CMD_EEPROM_READ		0xE1
#define COMPASS_CMD_EEPROM_WRITE	0xF1

Module_Compass::Module_Compass(QString id, Module_UID *uid)
    : RobotModule(id)
{
    this->uid=uid;

    setDefaultValue("i2cAddress", 0x19);
    setDefaultValue("frequency", 1);
    setDefaultValue("orientation","level");
    setDefaultValue("devAngle",0);
    setDefaultValue("varAngle",0);
    setDefaultValue("iirFilter",0);
    setDefaultValue("sampleRate",5);

    connect(&timer,SIGNAL(timeout()), this, SLOT(refreshData()));

    reset();
}

Module_Compass::~Module_Compass()
{
}

void Module_Compass::terminate()
{
    RobotModule::terminate();
    timer.stop();
}

void Module_Compass::reset()
{
    RobotModule::reset();

    if (!getSettings().value("enabled").toBool())
        return;

    int freq = 1000/getSettings().value("frequency").toInt();
    if (freq>0)
        timer.start(freq);
    else
        timer.stop();

    configure();
    resetDevice();
    printEEPROM();

}

void Module_Compass::configure()
{
    if (!getSettings().value("enabled").toBool())
        return;

    uint8_t op1 = 0;
    uint8_t op2 = 0;

    QString orientation = settings.value("orientation").toString();
    if (orientation == "level") {
        op1 |= 0x01;
    } else if (orientation == "upright edge") {
        op1 |= 0x02;
    } else if (orientation == "upright front") {
        op1 |= 0x04;
    } else {
        setHealthToSick("Bad config: "+orientation+" is not a valid orientation!");
    }

    int iir = settings.value("iirFilter").toInt();
    if (iir>0)
        op1 |= 0x20;

    int sampleRate = settings.value("sampleRate").toInt();
    switch (sampleRate) {
    case 1:
        break;
    case 5:
        op2 |= 0x01;
        break;
    case 10:
        op2 |= 0x02;
        break;
    default:
        setHealthToSick("Bad config: "+QString::number(sampleRate)+" is not a valid sample rate!");
    }
    if (iir>0)
        op1 |= 0x20;

    eepromWrite(0x04, op1);
    eepromWrite(0x05, op2);

    if (iir>=0 && iir<16)
        eepromWrite(0x14,(uint8_t)iir);

    signed short deviation = settings.value("devAngle").toInt();
    eepromWrite(0x0A, (uint8_t)(deviation & 0x00FF));
    eepromWrite(0x0B, (uint8_t)(deviation>>8 & 0x00FF));
    signed short variation = settings.value("varAngle").toInt();
    eepromWrite(0x0C, (uint8_t)(variation & 0x00FF));
    eepromWrite(0x0D, (uint8_t)(variation>>8 & 0x00FF));

}

void Module_Compass::refreshData()
{
    if (!getSettings().value("enabled").toBool())
        return;

    updateHeadingData();
    updateAccelData();
    updateMagData();
    updateStatusRegister();

    if (getHealthStatus().isHealthOk()) {
        emit dataChanged(this);
    }
}

QList<RobotModule*> Module_Compass::getDependencies()
{
    QList<RobotModule*> ret;
    ret.append(uid);
    return ret;
}

QWidget* Module_Compass::createView(QWidget* parent)
{
    return new Compass_Form(this, parent);
}

void Module_Compass::doHealthCheck()
{
    if (!getSettings().value("enabled").toBool())
        return;

    uint8_t sw_version = eepromRead(0x02);

    // dunno if newer chips still have this version
    if (sw_version != 7)
    {
        setHealthToSick("chip reports wrong software version: "+QString::number(sw_version));

    } else {
        setHealthToOk();
    }
}

unsigned short Module_Compass::toShort(uint8_t high, uint8_t low)
{
        return ((unsigned short)high) << 8 | ((unsigned short)low);
}

void Module_Compass::updateHeadingData()
{
    uint8_t recv_buffer[6];

    uint8_t send_buffer[1];
    send_buffer[0] = COMPASS_CMD_HEADING_DATA;

    if (!readWriteDelay(send_buffer, 1, recv_buffer, 6, 1)) {
        logger->error("Could not read heading data!");
    } else {
        data["heading"] = toShort(recv_buffer[0], recv_buffer[1])/10.0;
        data["pitch"] = (signed short)toShort(recv_buffer[2], recv_buffer[3])/10.0;
        data["roll"] = (signed short)toShort(recv_buffer[4], recv_buffer[5])/10.0;
    }
}

void Module_Compass::updateStatusRegister()
{
    uint8_t recv_buffer[1];

    uint8_t send_buffer[1];
    send_buffer[0] = COMPASS_CMD_GET_OPMODE;

    if (!readWriteDelay(send_buffer, 1, recv_buffer, 1, 1)) {
        logger->error("Could not read op mode!");
    } else {
        data["reg_om1"] = recv_buffer[0];
    }
    //data["reg_om1_eep"] = eepromRead(0x04);
}

void Module_Compass::updateMagData(void)
{
        uint8_t recv_buffer[6];

        uint8_t send_buffer[1];
        send_buffer[0] = COMPASS_CMD_MAG_DATA;

        if (!readWriteDelay(send_buffer, 1, recv_buffer, 6, 1)) {
            logger->error("Could not read mag data!");
        } else {
            data["magX"] = (signed short)toShort(recv_buffer[0], recv_buffer[1]);
            data["magY"] = (signed short)toShort(recv_buffer[2], recv_buffer[3]);
            data["magZ"] = (signed short)toShort(recv_buffer[4], recv_buffer[5]);
        }

}

void Module_Compass::updateAccelData(void)
{
        uint8_t recv_buffer[6];

        uint8_t send_buffer[1];
        send_buffer[0] = COMPASS_CMD_ACCEL_DATA;

        if (!readWriteDelay(send_buffer, 1, recv_buffer, 6, 1)) {
                logger->error("Could not read accel data!");
        } else {
            data["accelX"] = ((signed short)toShort(recv_buffer[0], recv_buffer[1]))/1000.0;
            data["accelY"] = ((signed short)toShort(recv_buffer[2], recv_buffer[3]))/1000.0;
            data["accelZ"] = ((signed short)toShort(recv_buffer[4], recv_buffer[5]))/1000.0;
        }

}


void Module_Compass::printEEPROM()
{
        for(uint8_t b = 0x00; b<=0x15; b++)
        {
                uint8_t content = eepromRead(b);
                logger->debug("EEPROM content at address 0x" + QString::number(b,16) + ": 0x" + QString::number(content,16));
        }
}

uint8_t Module_Compass::eepromRead(uint8_t addr)
{
        uint8_t recv_buffer[1];
        uint8_t send_buffer[2];
        send_buffer[0] = COMPASS_CMD_EEPROM_READ;
        send_buffer[1] = addr;
        readWriteDelay( send_buffer, 2, recv_buffer, 1, 10);
        return recv_buffer[0];
}

void Module_Compass::eepromWrite(uint8_t addr, uint8_t data)
{
    logger->debug("Setting eeprom address 0x"+QString::number(addr,16)+" to 0x"+QString::number(data,16));
    uint8_t current_value = eepromRead(addr);

    if (current_value != data) {
        logger->debug("Current value is 0x"+QString::number(current_value,16)+", updating eeprom.");
        uint8_t recv_buffer[1];
        uint8_t send_buffer[3];
        send_buffer[0] = COMPASS_CMD_EEPROM_WRITE;
        send_buffer[1] = addr;
        send_buffer[2] = data;
        if (!readWriteDelay(send_buffer, 3, recv_buffer, 0, 10)) {
            setHealthToSick("Could not write to EEPROM addr=0x" + QString::number(addr,16));
        }
    }
}

void Module_Compass::setOrientation()
{
        uint8_t recv_buffer[1];
        uint8_t send_buffer[1];
        QString orientation = settings.value("orientation").toString();
        if (orientation == "level")
            send_buffer[0] = COMPASS_CMD_ORIENT_LEVEL;
        else if (orientation == "upright edge")
            send_buffer[0] = COMPASS_CMD_ORIENT_SIDEWAYS;
        else if (orientation == "upright front")
            send_buffer[0] = COMPASS_CMD_ORIENT_FLAT;
        else {
            setHealthToSick("Bad config: "+orientation+" is not a valid orientation!");
            return;
        }

        if (!readWriteDelay(send_buffer, 1, recv_buffer, 0, 1)) {
                setHealthToSick("Could not set orientation!");
        }
}

void Module_Compass::startCalibration()
{
    if (!getSettings().value("enabled").toBool())
        return;

        uint8_t recv_buffer[1];
        uint8_t send_buffer[1];
        send_buffer[0] = COMPASS_CMD_CALIB_START;
        if (!readWriteDelay(send_buffer, 1, recv_buffer, 0, 1)) {
               setHealthToSick("Could not start calibration!");
        }
}

void Module_Compass::stopCalibration()
{
    if (!getSettings().value("enabled").toBool())
        return;

        uint8_t recv_buffer[1];
        uint8_t send_buffer[1];
        send_buffer[0] = COMPASS_CMD_CALIB_STOP;
        if (!readWriteDelay(send_buffer, 1, recv_buffer, 0, 1)) {
            setHealthToSick("Could not stop calibration!");
        }
}

void Module_Compass::resetDevice()
{
        uint8_t recv_buffer[1];
        uint8_t send_buffer[1];
        send_buffer[0] = COMPASS_CMD_SOFT_RESET;
        if (!readWriteDelay(send_buffer, 1, recv_buffer, 0, 1)) {
            setHealthToSick("Could not reset compass!");
        }
        sleep(500); // necessary waiting period
}

bool Module_Compass::readWriteDelay(unsigned char *send_buf, int send_size,
                                    unsigned char *recv_buf, int recv_size, int delay)
{
    unsigned char address = getSettings().value("i2cAddress").toInt();

    if (!uid->I2C_Write(address, send_buf, send_size)) {
        setHealthToSick("UID reported error during write.");
        return false;
    }
    sleep(delay);
    if (recv_size>0 && !uid->I2C_Read(address, recv_size, recv_buf)) {
        setHealthToSick("UID reported error during read.");
        return false;
    }
    return true;
}

float Module_Compass::getHeading()
{
    return data["heading"].toFloat();
}

float Module_Compass::getPitch()
{
    return data["pitch"].toFloat();
}

float Module_Compass::getRoll()
{
    return data["roll"].toFloat();
}
