#include "module_pressuresensor.h"
#include "pressure_form.h"
#include "module_uid.h"

#define REGISTER_CALIB 00 // 8 bytes
#define REGISTER_PRESSURE_RAW 8
#define REGISTER_TEMP_RAW 10
#define REGISTER_PRESSURE 12
#define REGISTER_TEMP 14
#define REGISTER_STATUS 17

// indicates problem between i2c-spi bridge and pressure sensor
#define STATUS_MAGIC_VALUE 0x55
#define CALIB_MAGIC_VALUE 224

// pressure range. everything outside this range will be regarded as
// a meassurement error
#define PRESSURE_MIN 900
#define PRESSURE_MAX 3000

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
    RobotModule::terminate();
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

    if (getHealthStatus().isHealthOk())
        emit dataChanged(this);
}

void Module_PressureSensor::readPressure()
{
    unsigned char readBuffer[2];

    if (!readRegister(REGISTER_PRESSURE, 2, readBuffer)) {
        setHealthToSick("UID reported error.");
        return;
    }

    // this is the pressure in mBar
    uint16_t pressure = (int)readBuffer[0] << 8 | (int)readBuffer[1];

    data["pressure"] =  pressure;

    // 100 mBar == ca. 1m wassersäule - druck an der luft
    data["depth"] =  ((float)pressure-getSettings().value("airPressure").toFloat())/100;

    if (pressure < PRESSURE_MIN || pressure > PRESSURE_MAX) {
        setHealthToSick("Pressure of "+QString::number(pressure) + " doesn't make sense.");
    }

}

void Module_PressureSensor::readTemperature()
{

    unsigned char readBuffer[2];
    if (!readRegister(REGISTER_TEMP, 2, readBuffer)) {
        setHealthToSick("UID reported error.");
        return;
    }

    // this is the temperature in 10/degree celsius
    uint16_t temp = (int)readBuffer[0] << 8 | (int)readBuffer[1];

    data["temperature"] = ((float)temp)/10;
}

float Module_PressureSensor::getDepth()
{
    float p = data["pressure"].toFloat();
    if (p > 900 && p <= 2000) {
        return data["depth"].toFloat();
    } else {
        setHealthToSick("Pressure of "+QString::number(p) + " doesn't make sense.");
        return 0;
    }

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

    unsigned char readBuffer[1];

    if (!readRegister(REGISTER_STATUS, 1, readBuffer)) {
        setHealthToSick("UID reported error.");
        return;
    }
    if (readBuffer[0] != STATUS_MAGIC_VALUE) {
        setHealthToSick("Status register doesn't match magic value: is="+QString::number(readBuffer[0]));
        return;
    }

    setHealthToOk();
}

bool Module_PressureSensor::readRegister2(unsigned char reg, int size, unsigned char *ret_buf)
{
    unsigned char address = getSettings().value("i2cAddress").toInt();

    if (!uid->getUID()->I2C_Write(address, &reg, 1)) {
        setHealthToSick("UID reported error.");
        return false;
    }
    if (!uid->getUID()->I2C_Read(address, size, ret_buf)) {
        setHealthToSick("UID reported error.");
        return false;
    }
    return true;
}

bool Module_PressureSensor::readRegister(unsigned char reg, int size, unsigned char *ret_buf)
{
    unsigned char address = getSettings().value("i2cAddress").toInt();

    if (!uid->getUID()->I2C_ReadRegisters(address, reg, size, ret_buf)) {
        setHealthToSick("UID reported error.");
        return false;
    }
    return true;
}
