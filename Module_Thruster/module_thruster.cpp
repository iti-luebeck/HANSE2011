#include "module_thruster.h"
#include "thruster_form.h"
#include "module_uid.h"


Module_Thruster::Module_Thruster(QString id, Module_UID *uid)
    : RobotModule(id)
{
    this->uid=uid;

    setDefaultValue("i2cAddress", 0x01);
    setDefaultValue("channel", 1);
    setDefaultValue("multiplicator", 127);

    initController();
}

void Module_Thruster::initController()
{
    unsigned char sendValue[] = { 0x01 };
    unsigned char address = getSettings().value("i2cAddress").toInt();

    bool ret = uid->getUID()->I2C_WriteRegister(address,0x00,sendValue,0x01);
    if (!ret)
        setHealthToSick("UID reported error.");
    else
        setHealthToOk();
}

Module_Thruster::~Module_Thruster()
{
}

void Module_Thruster::terminate()
{
}

void Module_Thruster::reset()
{
    // TODO: what exactly does this do?
    initController();
}

void Module_Thruster::setSpeed(float speed)
{
    if (!getSettings().value("enabled").toBool())
        return;

    data["speed"] = speed;

    if (speed > 1)
        speed = 1;

    if (speed < -1)
        speed = -1;

    unsigned char sendValue[] = { (int)(speed * getSettings().value("multiplicator").toInt()) };
    unsigned char address = getSettings().value("i2cAddress").toInt();
    unsigned char channel = getSettings().value("channel").toInt();

    bool ret = uid->getUID()->I2C_WriteRegister(address,channel,sendValue,0x01);
    if (!ret)
        setHealthToSick("UID reported error.");
    else
        setHealthToOk();

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
    if (uid->getUID()->I2C_Scan().contains(address))
        setHealthToOk();
    else
        setHealthToSick("Couldn't find i2c slave on bus.");
}
