#include "module_pressuresensor.h"
#include "pressure_form.h"
#include "module_uid.h"

#define REGISTER_PRESSURE 0x12
#define REGISTER_TEMP 0x14
#define REGISTER_CALIB 0x01

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

    unsigned char address = getSettings().value("i2cAddress").toInt();

    unsigned char readBuffer[2];
    bool ret = uid->getUID()->I2C_ReadRegisters(address, REGISTER_PRESSURE, 2, readBuffer);
    if (!ret) {
        setHealthToSick("UID reported error.");
        return;
    }

    // TODO: convert byte array to int

}

float Module_PressureSensor::getDepth()
{
    return depth;
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

