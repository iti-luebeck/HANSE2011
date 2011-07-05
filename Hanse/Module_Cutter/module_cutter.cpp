#include <Module_Cutter/module_cutter.h>
#include <Module_Cutter/form_cutter.h>


Module_Cutter::Module_Cutter(QString id, Module_UID *uid):RobotModule(id)
{
    this->uid = uid;

    setDefaultValue("i2cAddress", 0x01);
    setDefaultValue("timeout",50);

    isClosed = false;
    setEnabled(false);

    timer.moveToThread(this);
}

QList<RobotModule*> Module_Cutter::getDependencies()
{
    QList<RobotModule*> ret;
    ret.append(this->uid);
    return ret;
}

QWidget* Module_Cutter::createView(QWidget *parent)
{
    return new FormCutter(this, parent);
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
    connect(this, SIGNAL(enabled(bool)), this, SLOT(gotEnabled(bool)));
    timer.start(this->getSettingsValue("timeout").toInt());
}

void Module_Cutter::terminate()
{
    timer.stop();
    RobotModule::terminate();
}

void Module_Cutter::gotEnabled(bool value)
{
    Q_UNUSED(value);
//    if(value)
//    {
//        timer.start(this->getSettingsValue("timeout").toInt());
//    }
//    else
//    {
//        timer.stop();
//    }

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
    else if (data[0] != 2)
        setHealthToSick("sw revision register doesn't match magic value: is="+QString::number(data[0]));
    else
        setHealthToOk();

    for (int i=0; i<20; i++) {
        char data[1];
        bool ret = uid->I2C_ReadRegisters(address,i,1,data);
        addData("reg"+QString::number(i),data[0]);
        if (!ret)
            setHealthToSick(uid->getLastError());
    }
}

void Module_Cutter::closeCutter()
{
    char cutterPosition[] = { 1 };
    bool ret = uid->I2C_WriteRegister(this->getSettingsValue("i2cAddress").toInt(),0x01,cutterPosition,1);
    if(!ret)
        setHealthToSick("could not write register");
    addData("position", cutterPosition[0]);
}

void Module_Cutter::openCutter()
{
    char cutterPosition[] = { 60 };
    bool ret = uid->I2C_WriteRegister(this->getSettingsValue("i2cAddress").toInt(),0x01,cutterPosition,1);
    if(!ret)
        setHealthToSick("could not write register");
    addData("position", cutterPosition[0]);
}

void Module_Cutter::changeCutDirection()
{
    if(!this->isEnabled())
    {
        if(isClosed)
        {
            openCutter();
            isClosed = false;
        }
        return;
    }
    if(isClosed)
    {
        openCutter();
        isClosed = false;
    } else {
        closeCutter();
        isClosed = true;
    }
}
