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
    RobotModule::reset();

    int freq = 1000/getSettings().value("frequency").toInt();
    if (freq>0)
        timer.start(freq);
    else
        timer.stop();

    if (!getSettings().value("enabled").toBool())
        return;

    readCalibWords();

}

void Module_PressureSensor::refreshData()
{
    if (!getSettings().value("enabled").toBool())
        return;

    readPressure();
    readTemperature();
    readCounter();
    readRawRegisters();
    calc();

    if (getHealthStatus().isHealthOk()) {
        emit dataChanged(this);
        emit newDepthData(getDepth());
    }
}

void Module_PressureSensor::calc()
{
    short D1 = data["pressureRaw"].toInt();
    short D2 = data["tempRaw"].toInt();
    short C1 = data["C1"].toInt();
    short C2 = data["C2"].toInt();
    short C3 = data["C3"].toInt();
    short C4 = data["C4"].toInt();
    short C5 = data["C5"].toInt();
    short C6 = data["C6"].toInt();

    int UT1, dT, OFF, SENS, P;
    short int dT2;

    // Calculate calibration temperature
    UT1 = 8*C5+10000;

    // Calculate actual temperature
    dT = D2 - UT1;

    // Second-order temperature compensation
    if (dT < 0)
            dT2 = dT - (dT/128*dT/128)/2;
    else
            dT2 = dT - (dT/128*dT/128)/8;
    data["tempSW"] = 200+dT2*(C6+100)/2048;

    // Calculate temperature compensated pressure
    OFF = C2+((C4-250)*dT)/4096+10000;

    SENS = C1/2 + ((C3+200)*dT)/8192 + 3000;

    // Temperature compensated pressure in mbar
    P = (SENS*(D1-OFF))/4096+1000;

    data["pressureSW"] = P;
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

void Module_PressureSensor::readCounter()
{
    unsigned char readBuffer[1];

    if (!readRegister(REGISTER_COUNTER, 1, readBuffer)) {
        setHealthToSick("UID reported error.");
        return;
    }

    data["counter"] =  readBuffer[0];
}

void Module_PressureSensor::readCalibWords()
{
    unsigned char readBuffer[8];
    if (!readRegister(0, 8, readBuffer)) {
        setHealthToSick("UID reported error.");
        return;
    }

    uint16_t pressure_CalibrationWords[4];
    for(int i=0;i<4;i++) {

        // this is the temperature in 10/degree celsius
        uint16_t c = (int)readBuffer[i] << 8 | (int)readBuffer[i+1];

        data["calib"+i] = c;
        pressure_CalibrationWords[i]=c;
    }

    data["C1"] = ((pressure_CalibrationWords[0] & 0xFFF8) >> 3);
    data["C2"] = ((pressure_CalibrationWords[0] & 0x0007) << 10) + ((pressure_CalibrationWords[1] & 0xFFC0) >> 6);
    data["C3"] = ((pressure_CalibrationWords[2] & 0xFFC0) >> 6);
    data["C4"] = ((pressure_CalibrationWords[3] & 0xFF80) >> 7);
    data["C5"] = ((pressure_CalibrationWords[1] & 0x003F) << 6) + ((pressure_CalibrationWords[2] & 0x003F));
    data["C6"] = (pressure_CalibrationWords[3] & 0x007F);

}

void Module_PressureSensor::readRawRegisters()
{

    unsigned char readBuffer[2];
    if (!readRegister(REGISTER_TEMP_RAW, 2, readBuffer)) {
        setHealthToSick("UID reported error.");
        return;
    }

    uint16_t temp = (int)readBuffer[0] << 8 | (int)readBuffer[1];

    data["tempRaw"] = temp;

    if (!readRegister(REGISTER_PRESSURE_RAW, 2, readBuffer)) {
        setHealthToSick("UID reported error.");
        return;
    }

    uint16_t pressure = (int)readBuffer[0] << 8 | (int)readBuffer[1];

    data["pressureRaw"] = pressure;
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

    if (!uid->I2C_Write(address, &reg, 1)) {
        setHealthToSick("UID reported error.");
        return false;
    }
    if (!uid->I2C_Read(address, size, ret_buf)) {
        setHealthToSick("UID reported error.");
        return false;
    }
    return true;
}

bool Module_PressureSensor::readRegister(unsigned char reg, int size, unsigned char *ret_buf)
{
    unsigned char address = getSettings().value("i2cAddress").toInt();

    if (!uid->I2C_ReadRegisters(address, reg, size, ret_buf)) {
        setHealthToSick("UID reported error.");
        return false;
    }
    return true;
}
