#include "module_thruster.h"
#include "thruster_form.h"
#include <Module_UID/module_uid.h>

/* see http://www.robot-electronics.co.uk/htm/md22tech.htm for details */
#define REG_MODE 0x00
#define REG_CHAN1 0x01
#define REG_CHAN2 0x02
#define REG_ACCEL 0x03
/* Regs 4-6: unused */
#define REG_SWREV 0x07

// expected value from the "software revision" register
#define MAGIC_SWREV 10

Module_Thruster::Module_Thruster(QString id, Module_UID *uid)
    : RobotModule(id)
{
    this->uid=uid;

    setDefaultValue("i2cAddress", 0x01);
    setDefaultValue("channel", 1);
    setDefaultValue("multiplicator", 127);

    connect(this,SIGNAL(enabled(bool)), this, SLOT(gotEnabled(bool)));

    initController();
}

void Module_Thruster::initController()
{
    if (!getSettings().value("enabled").toBool())
        return;

    unsigned char sendValue[] = { 0x01 };
    unsigned char address = getSettings().value("i2cAddress").toInt();

    bool ret = uid->I2C_WriteRegister(address,REG_MODE,sendValue,0x01);
    if (!ret)
        setHealthToSick(uid->getLastError());
    else
        setHealthToOk();
}

Module_Thruster::~Module_Thruster()
{
}

void Module_Thruster::terminate()
{
    RobotModule::terminate();
}

void Module_Thruster::reset()
{
    RobotModule::reset();

    initController();
}

void Module_Thruster::setSpeed(float speed)
{
    if (!getSettings().value("enabled").toBool())
        return;

    if (speed > 1)
        speed = 1;

    if (speed < -1)
        speed = -1;


    int speedRaw = (int)(speed * getSettings().value("multiplicator").toInt());
    data["speed"] = speedRaw;

    unsigned char sendValue[] = { speedRaw };
    unsigned char address = getSettings().value("i2cAddress").toInt();
    unsigned char channel = getSettings().value("channel").toInt();

    if (channel != 1 && channel != 2) {
        setHealthToSick("thruster configured to an illegal channel.");
        return;
    }

    bool ret = uid->I2C_WriteRegister(address,channel,sendValue,0x01);
    if (!ret)
        setHealthToSick(uid->getLastError());
    else {
        setHealthToOk();
        emit dataChanged(this);
    }

}

QList<RobotModule*> Module_Thruster::getDependencies()
{
    QList<RobotModule*> ret;
    ret.append(uid);
    return ret;
}

QWidget* Module_Thruster::createView(QWidget* parent)
{
    return new Thruster_Form(this, parent);
}

void Module_Thruster::doHealthCheck()
{
    if (!getSettings().value("enabled").toBool())
        return;

    int address = getSettings().value("i2cAddress").toInt();
    unsigned char data[1];
    bool ret = uid->I2C_ReadRegisters(address,REG_SWREV,1,data);
    if (!ret)
        setHealthToSick(uid->getLastError());
    else if (data[0] != MAGIC_SWREV)
        setHealthToSick("sw revision register doesn't match magic value: is="+QString::number(data[0]));
    else
        setHealthToOk();
}

void Module_Thruster::gotEnabled(bool value)
{
    if (value)
        initController();
}
