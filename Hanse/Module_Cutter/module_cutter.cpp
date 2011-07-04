#include <Module_Cutter/module_cutter.h>


Module_Cutter::Module_Cutter(QString id, Module_UID *uid):RobotModule(id)
{
    this->uid = uid;

    setDefaultValue("i2cAddress", 0x01);
    setDefaultValue("timeout",50);

    timer.moveToThread(this);
}

QList<RobotModule*> Module_Cutter::getDependencies()
{
    QList<RobotModule*> ret;
    ret.append(this->uid);
    return ret;
}

bool Module_Cutter::shutdown()
{
    timer.stop();
    return RobotModule::shutdown();
}

void Module_Cutter::reset()
{
    timer.stop();
    RobotModule::reset();
    timer.start(this->getSettingsValue("timeout").toInt());
}

void Module_Cutter::init()
{
    connect(&timer,SIGNAL(timeout()),this,SLOT(changeCutDirection()));
}

void Module_Cutter::terminate()
{
    timer.stop();
    RobotModule::terminate();
}

void Module_Cutter::setEnabled(bool value)
{
    if(value)
    {
        timer.start(this->getSettingsValue("timeout").toInt());
    }
    else
    {
        timer.stop();
    }

}

void Module_Cutter::doHealthCheck()
{
    if (!getSettingsValue("enabled").toBool())
        return;

    int address = getSettingsValue("i2cAddress").toInt();
    char data[1];
    bool ret = uid->I2C_ReadRegisters(address,0x00,1,data);
    addData("rev",data[0]);
    if (!ret)
        setHealthToSick(uid->getLastError());
    else if (data[0] != 3)
        setHealthToSick("sw revision register doesn't match magic value: is="+QString::number(data[0]));
    else
        setHealthToOk();
}

void Module_Cutter::changeCutDirection()
{
    if(!this->isEnabled())
    {
        timer.stop();
        return;
    }
    if(cutterPosition[0] == 1)
    {
        cutterPosition[0] = 136;
    }
    else
    {
        cutterPosition[0] = 1;
    }
    bool ret = uid->I2C_WriteRegister(this->getSettingsValue("ic2Address").toInt(),(char)0x01,(const char*) cutterPosition,1);
    if(!ret)
        setHealthToSick("could not write register");
    else
        setHealthToOk();
}
