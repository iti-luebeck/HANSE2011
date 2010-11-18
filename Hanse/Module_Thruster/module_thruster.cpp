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
    : RobotModule_MT(id)
{
    this->uid=uid;

    qDebug() << "lock1";
    setDefaultValue("i2cAddress", 0x01);
    setDefaultValue("channel", 1);
    setDefaultValue("multiplicator", 127);
qDebug() << "unlock1";
    connect(this,SIGNAL(enabled(bool)), this, SLOT(gotEnabled(bool)));

    initController();
}

void Module_Thruster::initController()
{
    if (!getSettingsValue("enabled").toBool())
        return;

    char sendValue[] = { 0x01 };
    qDebug() << "lock2";
    unsigned char address = getSettingsValue("i2cAddress").toInt();
qDebug() << "unlock2";
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
    setSpeed(0);
//    QTimer::singleShot(0,this,SLOT(stop()));
    RobotModule_MT::terminate();
}

void Module_Thruster::reset()
{
    RobotModule::reset();

    initController();

    setSpeed(0);
}

void Module_Thruster::stop()
{
    setSpeed(0);
}

void Module_Thruster::setSpeed(float speed)
{
//        qDebug() << "press ref THREAD ID";
//        qDebug() << QThread::currentThreadId();

    qDebug() << "lock3";

    if (!getSettingsValue("enabled").toBool())
        return;
this->dataLockerMutex.lock();

//    this->dataLockerMutex.lock();

    if (speed > 1)
        speed = 1;

    if (speed < -1)
        speed = -1;


    int speedRaw = (int)(speed * getSettingsValue("multiplicator").toInt());
qDebug() << "lock3_1";
    addData("speed", speedRaw);
qDebug() << "unlock3_1";
    char sendValue[] = { speedRaw };
    qDebug() << "lock3_2";
    unsigned char address = getSettingsValue("i2cAddress").toInt();
    qDebug() << "unlock3_2";
    qDebug() << "lock3_3";
    unsigned char channel = getSettingsValue("channel").toInt();
    qDebug() << "unlock3_3";

    if (channel != 1 && channel != 2) {
        setHealthToSick("thruster configured to an illegal channel.");
        return;
    }

    bool ret = uid->I2C_WriteRegister(address,channel,sendValue,0x01);
    this->dataLockerMutex.unlock();
    qDebug() << "unlock3";
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

    if (!getSettingsValue("enabled").toBool())
        return;

qDebug() << "lock4";
    int address = getSettingsValue("i2cAddress").toInt();
    qDebug() << "unlock4";
    char data[1];
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
    else
        setSpeed(0);
}
