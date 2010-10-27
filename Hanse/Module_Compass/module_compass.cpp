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
    : RobotModule_MT(id)
{
    thread.start();

    this->uid=uid;
    QObject::connect(this,SIGNAL(I2C_Read(unsigned char,short,char*,bool)),uid,SLOT(I2C_Read(unsigned char,short,char*,bool)),Qt::BlockingQueuedConnection);
    QObject::connect(this,SIGNAL(getUIDErrorMsg(QString)),uid,SLOT(getLastError(QString)),Qt::BlockingQueuedConnection);

    setDefaultValue("i2cAddress", 0x19);
    setDefaultValue("frequency", 1);
    setDefaultValue("orientation","level");
    setDefaultValue("devAngle",0);
    setDefaultValue("varAngle",0);
    setDefaultValue("iirFilter",0);
    setDefaultValue("sampleRate",5);
    setDefaultValue("debug",1);

    //timer.moveToThread(&thread);
    thread.start();
    connect(&timer,SIGNAL(timeout()), this, SLOT(refreshData()),
            Qt::DirectConnection);

    reset();
}

Module_Compass::~Module_Compass()
{
}

void Module_Compass::terminate()
{
    RobotModule::terminate();
    QTimer::singleShot(0, &timer, SLOT(stop()));
}

void Module_Compass::reset()
{
    RobotModule::reset();

    if (!this->getSettingsValue("enabled").toBool())
        return;

    int freq = 1000/this->getSettingsValue("frequency").toInt();
    if (freq>0) {
        timer.setInterval(freq);
        QTimer::singleShot(0, &timer, SLOT(start()));
    } else {
        QTimer::singleShot(0, &timer, SLOT(stop()));
    }
    configure();
    resetDevice();
    printEEPROM();

}

void Module_Compass::configure()
{
    if (!this->getSettingsValue("enabled").toBool())
        return;

    uint8_t op1 = 0;
    uint8_t op2 = 0;

    QString orientation = this->getSettingsValue("orientation").toString();
    if (orientation == "level") {
        op1 |= 0x01;
    } else if (orientation == "upright edge") {
        op1 |= 0x02;
    } else if (orientation == "upright front") {
        op1 |= 0x04;
    } else {
        setHealthToSick("Bad config: "+orientation+" is not a valid orientation!");
    }

    int iir = this->getSettingsValue("iirFilter").toInt();
    if (iir>0)
        op1 |= 0x20;

    int sampleRate = this->getSettingsValue("sampleRate").toInt();
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
    if (!this->getSettingsValue("enabled").toBool())
        return;

    updateHeadingData();
    if (this->getSettingsValue("debug").toBool()) {
        updateAccelData();
        updateMagData();
        updateStatusRegister();
    }

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

    uint8_t sw_version;
    if (!eepromRead(0x02,sw_version)) {
        QString err;
        emit getUIDErrorMsg(err);
        setHealthToSick(err);
        return;
    }

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
    char recv_buffer[6];

    char send_buffer[1];
    send_buffer[0] = COMPASS_CMD_HEADING_DATA;

    if (!readWriteDelay(send_buffer, 1, recv_buffer, 6, 1)) {
        logger->error("Could not read heading data!");
    } else {
        QMutexLocker l(&moduleMutex);
        this->addData("heading",toShort(recv_buffer[0], recv_buffer[1])/10.0);
        this->addData("pitch",(signed short)toShort(recv_buffer[2], recv_buffer[3])/10.0);
        this->addData("roll",(signed short)toShort(recv_buffer[4], recv_buffer[5])/10.0);
//        data["heading"] = toShort(recv_buffer[0], recv_buffer[1])/10.0;
//        data["pitch"] = (signed short)toShort(recv_buffer[2], recv_buffer[3])/10.0;
//        data["roll"] = (signed short)toShort(recv_buffer[4], recv_buffer[5])/10.0;
        emit newHeading(this->getDataValue("heading").toFloat());
        emit newPitch(this->getSettingsValue("pitch").toFloat());
        emit newRoll(this->getDataValue("roll").toFloat());
    }
}

void Module_Compass::updateStatusRegister()
{
    char recv_buffer[1];

    char send_buffer[1];
    send_buffer[0] = COMPASS_CMD_GET_OPMODE;

    if (!readWriteDelay(send_buffer, 1, recv_buffer, 1, 1)) {
        logger->error("Could not read op mode!");
    } else {
        QMutexLocker l(&moduleMutex);
        this->addData("reg_om1", recv_buffer[0]);
    }
    //data["reg_om1_eep"] = eepromRead(0x04);
}

void Module_Compass::updateMagData(void)
{
        char recv_buffer[6];

        char send_buffer[1];
        send_buffer[0] = COMPASS_CMD_MAG_DATA;

        if (!readWriteDelay(send_buffer, 1, recv_buffer, 6, 1)) {
            logger->error("Could not read mag data!");
        } else {
            QMutexLocker l(&moduleMutex);
            this->addData("magX",(signed short)toShort(recv_buffer[0], recv_buffer[1]));
            this->addData("magY",(signed short)toShort(recv_buffer[2], recv_buffer[3]));
            this->addData("magZ",(signed short)toShort(recv_buffer[4], recv_buffer[5]));
//            data["magX"] = (signed short)toShort(recv_buffer[0], recv_buffer[1]);
//            data["magY"] = (signed short)toShort(recv_buffer[2], recv_buffer[3]);
//            data["magZ"] = (signed short)toShort(recv_buffer[4], recv_buffer[5]);
        }

}

void Module_Compass::updateAccelData(void)
{
        char recv_buffer[6];

        char send_buffer[1];
        send_buffer[0] = COMPASS_CMD_ACCEL_DATA;

        if (!readWriteDelay(send_buffer, 1, recv_buffer, 6, 1)) {
                logger->error("Could not read accel data!");
        } else {
            QMutexLocker l(&moduleMutex);
            this->addData("accelX",((signed short)toShort(recv_buffer[0], recv_buffer[1]))/1000.0);
            this->addData("accelY",((signed short)toShort(recv_buffer[2], recv_buffer[4]))/1000.0);
            this->addData("accelZ",((signed short)toShort(recv_buffer[4], recv_buffer[5]))/1000.0);
//            data["accelX"] = ((signed short)toShort(recv_buffer[0], recv_buffer[1]))/1000.0;
//            data["accelY"] = ((signed short)toShort(recv_buffer[2], recv_buffer[3]))/1000.0;
//            data["accelZ"] = ((signed short)toShort(recv_buffer[4], recv_buffer[5]))/1000.0;
        }

}


void Module_Compass::printEEPROM()
{
        for(uint8_t b = 0x00; b<=0x15; b++)
        {
                uint8_t content;
                bool ret = eepromRead(b, content);
                if (ret)
                    logger->debug("EEPROM content at address 0x" + QString::number(b,16) + ": 0x" + QString::number(content,16));
        }
}

bool Module_Compass::eepromRead(uint8_t addr, uint8_t &data)
{
        char recv_buffer[1];
        char send_buffer[2];
        send_buffer[0] = COMPASS_CMD_EEPROM_READ;
        send_buffer[1] = addr;
        bool ret = readWriteDelay( send_buffer, 2, recv_buffer, 1, 10);
        data = recv_buffer[0];
        return ret;
}

bool Module_Compass::eepromWrite(uint8_t addr, uint8_t data)
{
    logger->debug("Setting eeprom address 0x"+QString::number(addr,16)+" to 0x"+QString::number(data,16));
    uint8_t current_value;
    if (!eepromRead(addr,current_value))
        return false;

    if (current_value != data) {
        logger->debug("Current value is 0x"+QString::number(current_value,16)+", updating eeprom.");
        char recv_buffer[1];
        char send_buffer[3];
        send_buffer[0] = COMPASS_CMD_EEPROM_WRITE;
        send_buffer[1] = addr;
        send_buffer[2] = data;
        if (!readWriteDelay(send_buffer, 3, recv_buffer, 0, 10)) {
            return false;
            logger->error("Could not write to EEPROM addr=0x" + QString::number(addr,16));
        }
    }
    return true;
}

void Module_Compass::setOrientation()
{
        char recv_buffer[1];
        char send_buffer[1];
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
    if (!this->getSettingsValue("enabled").toBool())
        return;

        char recv_buffer[1];
        char send_buffer[1];
        send_buffer[0] = COMPASS_CMD_CALIB_START;
        if (!readWriteDelay(send_buffer, 1, recv_buffer, 0, 1)) {
               setHealthToSick("Could not start calibration!");
        }
}

void Module_Compass::stopCalibration()
{
    if (!this->getSettingsValue("enabled").toBool())
        return;

        char recv_buffer[1];
        char send_buffer[1];
        send_buffer[0] = COMPASS_CMD_CALIB_STOP;
        if (!readWriteDelay(send_buffer, 1, recv_buffer, 0, 1)) {
            setHealthToSick("Could not stop calibration!");
        }
}

void Module_Compass::resetDevice()
{
        char recv_buffer[1];
        char send_buffer[1];
        send_buffer[0] = COMPASS_CMD_SOFT_RESET;
        if (!readWriteDelay(send_buffer, 1, recv_buffer, 0, 1)) {
            setHealthToSick("Could not reset compass!");
        }
        msleep(500); // necessary waiting period
}

bool Module_Compass::readWriteDelay(char *send_buf, int send_size,
                                    char *recv_buf, int recv_size, int delay)
{
    char address = this->getSettingsValue("i2cAddress").toInt();
    bool status;
    emit I2C_Read(address,send_size,send_buf,status);
    if (!status) {
        QString err;
        emit getUIDErrorMsg(err);
        setHealthToSick(err);
        return false;
    }
    msleep(delay);
    emit I2C_Read(address, recv_size, recv_buf, status);
    if (recv_size>0 && !status) {
        QString err;
        emit getUIDErrorMsg(err);
        setHealthToSick(err);
        return false;
    }
    return true;
}

float Module_Compass::getHeading()
{
    QMutexLocker l(&moduleMutex);
    return this->getDataValue("heading").toFloat();
}

float Module_Compass::getPitch()
{
    QMutexLocker l(&moduleMutex);
    return this->getDataValue("pitch").toFloat();
}

float Module_Compass::getRoll()
{
    QMutexLocker l(&moduleMutex);
    return this->getDataValue("roll").toFloat();
}
