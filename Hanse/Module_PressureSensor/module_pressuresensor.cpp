#include "module_pressuresensor.h"
#include "pressure_form.h"
#include <Module_UID/module_uid.h>

#define REGISTER_CALIB 00 // 8 bytes
#define REGISTER_PRESSURE_RAW 8
#define REGISTER_TEMP_RAW 10
#define REGISTER_PRESSURE 12
#define REGISTER_TEMP 14
#define REGISTER_STATUS 17
#define REGISTER_COUNTER 20

#define REQUEST_NEW_CALIB_VALUES 18

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
    thread.start();

    this->uid=uid;

    setDefaultValue("i2cAddress", 0x50);
    setDefaultValue("frequency", 1);

    thread.start();
    //timer.moveToThread(&thread);

    connect(&timer,SIGNAL(timeout()), this, SLOT(refreshData()),
            Qt::DirectConnection);

    reset();
}

Module_PressureSensor::~Module_PressureSensor()
{
}

void Module_PressureSensor::terminate()
{
    RobotModule::terminate();
    QTimer::singleShot(0, &timer, SLOT(stop()));
}

void Module_PressureSensor::reset()
{
    RobotModule::reset();

    int freq = 1000/getSettings().value("frequency").toInt();
    if (freq>0) {
        timer.setInterval(freq);
        QTimer::singleShot(0, &timer, SLOT(start()));
    } else {
        QTimer::singleShot(0, &timer, SLOT(stop()));
    }

    if (!getSettings().value("enabled").toBool())
        return;

    unsigned char address = getSettings().value("i2cAddress").toInt();
    char reg = REQUEST_NEW_CALIB_VALUES;
    if (!uid->I2C_Write(address, &reg, 1)) {
        setHealthToSick(uid->getLastError());
    }
    msleep(100);

}

void Module_PressureSensor::refreshData()
{
    if (!getSettings().value("enabled").toBool())
        return;

    readPressure();
    readTemperature();

    if (getHealthStatus().isHealthOk()) {
        emit dataChanged(this);
        emit newDepthData(getDepth());
    }
}

void Module_PressureSensor::readPressure()
{
    unsigned char readBuffer[2];

    if (!readRegister(REGISTER_PRESSURE, 2, (char*)readBuffer)) {
        setHealthToSick(uid->getLastError());
        return;
    }

    // this is the pressure in mBar
    uint16_t pressure = (int)readBuffer[0] << 8 | (int)readBuffer[1];

    QMutexLocker l(&this->moduleMutex);

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
    if (!readRegister(REGISTER_TEMP, 2, (char*)readBuffer)) {
        setHealthToSick(uid->getLastError());
        return;
    }

    // this is the temperature in 10/degree celsius
    uint16_t temp = (int)readBuffer[0] << 8 | (int)readBuffer[1];

    QMutexLocker l(&this->moduleMutex);

    data["temperatureHW"] = ((float)temp)/10;
}

float Module_PressureSensor::getDepth()
{
    QMutexLocker l(&this->moduleMutex);
    return data["depth"].toFloat();
}

float Module_PressureSensor::getTemperature()
{
    QMutexLocker l(&this->moduleMutex);
    return data["temperature"].toFloat()/10.0;
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
    if (!isEnabled())
        return;

    char readBuffer[1];

    if (!readRegister(REGISTER_STATUS, 1, readBuffer)) {
        setHealthToSick(uid->getLastError());
        return;
    }

    if (readBuffer[0] != STATUS_MAGIC_VALUE) {
        setHealthToSick("Status register doesn't match magic value: is="+QString::number(readBuffer[0]));
        return;
    }

    // check if the counter is increasing.

    if (!readRegister(REGISTER_COUNTER, 1, readBuffer)) {
        setHealthToSick(uid->getLastError());
        return;
    }

    QMutexLocker l(&this->moduleMutex);

    data["counter"] = (unsigned char)readBuffer[0];

    if (readBuffer[0] == counter) {
        setHealthToSick("read the same counter value twice!");
        return;
    }

    counter = (unsigned char)readBuffer[0];

    setHealthToOk();
}

bool Module_PressureSensor::readRegister(unsigned char reg, int size, char *ret_buf)
{
    unsigned char address = getSettings().value("i2cAddress").toInt();

    if (!uid->I2C_ReadRegisters(address, reg, size, ret_buf)) {
        setHealthToSick(uid->getLastError());
        return false;
    }
    return true;
}
