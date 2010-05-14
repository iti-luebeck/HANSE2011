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
    if (!getSettings().value("enabled").toBool())
        return;

    RobotModule::reset();

    int freq = 1000/getSettings().value("frequency").toInt();
    if (freq>0)
        timer.start(freq);
    else
        timer.stop();

    resetDevice();
    sleep(500);

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

    uint8_t OP_REG_1 = eepromRead(0x04);

    uint8_t OP_REG_1_TRUE = 0x01 | 0x10;

    if (OP_REG_1 != OP_REG_1_TRUE)
    {
            logger->error("WARN: register 1 has bad content!");
            //printEEPROM();
    }

    setHealthToOk();
}

signed short Module_Compass::toSignedShort(uint8_t high, uint8_t low)
{
        return (signed short)toShort(high, low);
}

unsigned short Module_Compass::toShort(uint8_t high, uint8_t low)
{
        return ((unsigned short)high) << 8 | ((unsigned short)low);
}

signed int Module_Compass::getHeading(void)
{
        return data["heading"].toInt();
}

void Module_Compass::updateHeadingData()
{
    uint8_t recv_buffer[6];

    uint8_t send_buffer[1];
    send_buffer[0] = COMPASS_CMD_HEADING_DATA;

    if (!readWriteDelay(send_buffer, 1, recv_buffer, 6, 1)) {
        logger->error("Could not read heading data!");
    } else {
        data["heading"] = toShort(recv_buffer[0], recv_buffer[1]);
        data["pitch"] = toShort(recv_buffer[2], recv_buffer[3]);
        data["roll"] = toShort(recv_buffer[4], recv_buffer[5]);
    }
}

void Module_Compass::updateStatusRegister()
{
    data["reg_om1"] = eepromRead(0x04);
}

void Module_Compass::updateMagData(void)
{
        uint8_t recv_buffer[6];

        uint8_t send_buffer[1];
        send_buffer[0] = COMPASS_CMD_MAG_DATA;

        if (!readWriteDelay(send_buffer, 1, recv_buffer, 6, 1)) {
            logger->error("Could not read mag data!");
        } else {
            data["magX"] = toShort(recv_buffer[0], recv_buffer[1]);
            data["magY"] = toShort(recv_buffer[2], recv_buffer[3]);
            data["magZ"] = toShort(recv_buffer[4], recv_buffer[5]);
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
            data["accelX"] = toShort(recv_buffer[0], recv_buffer[1]);
            data["accelY"] = toShort(recv_buffer[2], recv_buffer[3]);
            data["accelZ"] = toShort(recv_buffer[4], recv_buffer[5]);
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
        uint8_t recv_buffer[1];
        uint8_t send_buffer[3];
        send_buffer[0] = COMPASS_CMD_EEPROM_WRITE;
        send_buffer[1] = addr;
        send_buffer[2] = data;
        if (!readWriteDelay(send_buffer, 3, recv_buffer, 0, 10)) {
            logger->error("Could not write to EEPROM addr=0x" + QString::number(addr,16));
        }}

void Module_Compass::setOrientation()
{
        uint8_t recv_buffer[1];
        uint8_t send_buffer[1];
        send_buffer[0] = COMPASS_CMD_ORIENT_LEVEL;
        if (!readWriteDelay(send_buffer, 1, recv_buffer, 0, 1)) {
                logger->error("Could not set orientation!");
        }
}

void Module_Compass::startCalibration()
{
        uint8_t recv_buffer[1];
        uint8_t send_buffer[1];
        send_buffer[0] = COMPASS_CMD_CALIB_START;
        if (!readWriteDelay(send_buffer, 1, recv_buffer, 0, 1)) {
               logger->error("Could not start calibration!");
        }
}

void Module_Compass::stopCalibration()
{
        uint8_t recv_buffer[1];
        uint8_t send_buffer[1];
        send_buffer[0] = COMPASS_CMD_CALIB_STOP;
        if (!readWriteDelay(send_buffer, 1, recv_buffer, 0, 1)) {
            logger->error("Could not stop calibration!");
        }
}

void Module_Compass::resetDevice()
{
        uint8_t recv_buffer[1];
        uint8_t send_buffer[1];
        send_buffer[0] = COMPASS_CMD_SOFT_RESET;
        if (!readWriteDelay(send_buffer, 1, recv_buffer, 0, 1)) {
            logger->error("Could not reset compass!");
        }
}

bool Module_Compass::readWriteDelay(unsigned char *send_buf, int send_size,
                                    unsigned char *recv_buf, int recv_size, int delay)
{
    unsigned char address = getSettings().value("i2cAddress").toInt();

    if (!uid->getUID()->I2C_Write(address, send_buf, send_size)) {
        setHealthToSick("UID reported error.");
        return false;
    }
    sleep(delay);
    if (recv_size>0 && !uid->getUID()->I2C_Read(address, recv_size, recv_buf)) {
        setHealthToSick("UID reported error.");
        return false;
    }
    return true;
}
