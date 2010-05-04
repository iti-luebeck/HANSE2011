#include "module_pressuresensor.h"
#include "pressure_form.h"
#include "module_uid.h"

#define REGISTER_PRESSURE 0x12
#define REGISTER_TEMP 0x14
#define REGISTER_CALIB 0x00 // 8 byte
#define REGISTER_STATUS 0x55 // TODO: correct!

#define STATUS_MAGIC_VALUE 0x10 // TODO: correct!

#define CALIB_MAGIC_VALUE 0x10 // TODO: correct!

Module_PressureSensor::Module_PressureSensor(QString id, Module_UID *uid)
    : RobotModule(id)
{
    this->uid=uid;

    setDefaultValue("i2cAddress", 0x50);
    setDefaultValue("frequency", 1);

    connect(&timer,SIGNAL(timeout()), this, SLOT(refreshData()));

    reset();
}

Module_PressureSensor::~Module_PressureSensor()
{
}

void Module_PressureSensor::terminate()
{
    timer.stop();
}

void Module_PressureSensor::reset()
{
    int freq = 1000/getSettings().value("frequency").toInt();
    if (freq>0)
        timer.start(freq);
    else
        timer.stop();
}

void Module_PressureSensor::refreshData()
{
    if (!getSettings().value("enabled").toBool())
        return;

    readPressure();
    readTemperature();
}

void Module_PressureSensor::readPressure()
{
    unsigned char address = getSettings().value("i2cAddress").toInt();

    unsigned char readBuffer[2];
    bool ret = uid->getUID()->I2C_ReadRegisters(address, REGISTER_PRESSURE, 2, readBuffer);
    if (!ret) {
        setHealthToSick("UID reported error.");
        return;
    }

    // this is the pressure in mBar
    uint16_t pressure = (int)readBuffer[0] << 8 | (int)readBuffer[1];

    // 100 mBar == ca. 1m wassersäule
    data["depth"] =  ((float)pressure)/100;

}

void Module_PressureSensor::readTemperature()
{
    unsigned char address = getSettings().value("i2cAddress").toInt();

    unsigned char readBuffer[2];
    bool ret = uid->getUID()->I2C_ReadRegisters(address, REGISTER_TEMP, 2, readBuffer);
    if (!ret) {
        setHealthToSick("UID reported error.");
        return;
    }

    // this is the temperature in 10/degree celsius
    uint16_t temp = (int)readBuffer[0] << 8 | (int)readBuffer[1];

    data["temperature"] = ((float)temp)/10;
}

float Module_PressureSensor::getDepth()
{
    return data["depth"].toFloat();
}

float Module_PressureSensor::getTemperature()
{
    return data["temperature"].toFloat();
}

QList<RobotModule*> Module_PressureSensor::getDependencies()
{
    QList<RobotModule*> ret;
    ret.append(uid);
    return ret;
}

QWidget* Module_PressureSensor::createView(QWidget* parent)
{
    return new Pressure_Form(this, parent);
}

void Module_PressureSensor::doHealthCheck()
{
    if (!getSettings().value("enabled").toBool())
        return;

    unsigned char address = getSettings().value("i2cAddress").toInt();

    unsigned char readBuffer[1];

    if (!uid->getUID()->I2C_ReadRegisters(address, REGISTER_CALIB, 1, readBuffer)) {
        setHealthToSick("UID reported error.");
        return;
    }
    if (readBuffer[0] != CALIB_MAGIC_VALUE) {
        setHealthToSick("First calibration byte doesn't match.");
        return;
    }

    if (!uid->getUID()->I2C_ReadRegisters(address, REGISTER_STATUS, 1, readBuffer)) {
        setHealthToSick("UID reported error.");
        return;
    }
    if (readBuffer[0] != STATUS_MAGIC_VALUE) {
        setHealthToSick("Status register doesn't match magic value.");
    }
}
