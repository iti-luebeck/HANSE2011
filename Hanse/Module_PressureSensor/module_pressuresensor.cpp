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

    unsigned char address = getSettings().value("i2cAddress").toInt();
    unsigned char reg = REQUEST_NEW_CALIB_VALUES;
    if (!uid->I2C_Write(address, &reg, 1)) {
        setHealthToSick("UID reported error while REQUEST_NEW_CALIB_VALUES.");
    }
    sleep(100);

    readCalibWords();

}

void Module_PressureSensor::refreshData()
{
    if (!getSettings().value("enabled").toBool())
        return;

    readPressure();
    readTemperature();
    readCounter();
    //readRawRegisters();
    //calc();

//    if (getHealthStatus().isHealthOk()) {
        emit dataChanged(this);
        emit newDepthData(getDepth());
//    }
}

void Module_PressureSensor::calc()
{
    float D1 = data["pressureRaw"].toInt();
    float D2 = data["tempRaw"].toInt();
//    short int C1 = data["C1"].toInt();
//    short int C2 = data["C2"].toInt();
//    short int C3 = data["C3"].toInt();
//    short int C4 = data["C4"].toInt();
//    short int C5 = data["C5"].toInt();
//    short int C6 = data["C6"].toInt();

    float UT1, dT, OFF, SENS, P;
    float dT2;

    // Calculate calibration temperature
    UT1 = 8*C5+10000;
    data["UT1"] = UT1;

    // Calculate actual temperature
    dT = D2 - UT1;
    data["dT"] = dT;

    // Second-order temperature compensation
    if (dT < 0)
            dT2 = dT - (dT/128.0*dT/128)/2.0;
    else
            dT2 = dT - (dT/128.0*dT/128)/8.0;
    data["tempSW"] = (200+dT2*(C6+100)/2048.0)/10.0;
    data["dT2"] = dT2;

    // Calculate temperature compensated pressure
    OFF = C2+((C4-250)*dT)/4096.0+10000;
    data["OFF"] = OFF;

    SENS = C1/2 + ((C3+200)*dT)/8192.0 + 3000;
    data["SENS"] = SENS;

    // Temperature compensated pressure in mbar
    P = (SENS*(D1-OFF))/4096.0+1000;

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

    data["pressureHW"] =  pressure;

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
    unsigned char address = getSettings().value("i2cAddress").toInt();
    unsigned char cmd[] = { 0x12 };
    if (!uid->I2C_Write(address, cmd, 1)) {
        setHealthToSick("could not reread calib  words.");
        sleep(100);
        return;
    }
    sleep(100);

    unsigned char readBuffer[8];
    if (!readRegister(0, 8, readBuffer)) {
        setHealthToSick("UID reported error.");
        return;
    }

    uint16_t pcW[4];
    for(int i=0;i<4;i++) {

        // this is the temperature in 10/degree celsius
        uint16_t c = (int)readBuffer[2*i] << 8 | (int)readBuffer[2*i+1];

        data["calib "+QString::number(i)] = "0x"+QString::number(c,16);
        pcW[i]=c;
    }

    C1 = ((pcW[0] & 0xFFF8) >> 3);
    C2 = ((pcW[0] & 0x0007) << 10) | ((pcW[1] & 0xFFC0) >> 6);
    C3 = ((pcW[2] & 0xFFC0) >> 6);
    C4 = ((pcW[3] & 0xFF80) >> 7);
    C5 = ((pcW[1] & 0x003F) << 6) | ((pcW[2] & 0x003F));
    C6 = (pcW[3] & 0x007F);

    data["C1"] = C1;
    data["C2"] = C2;
    data["C3"] = C3;
    data["C4"] = C4;
    data["C5"] = C5;
    data["C6"] = C6;
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

    data["temperatureHW"] = ((float)temp)/10;
}

float Module_PressureSensor::getDepth()
{
    return data["depth"].toFloat();
}

float Module_PressureSensor::getTemperature()
{
    return data["temperatureSW"].toFloat()/10.0;
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
