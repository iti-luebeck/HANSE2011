#include "behaviour_testmt.h"
#include <QtGui>
#include <Behaviour_TestMT/testmtform.h>

Behaviour_TestMT::Behaviour_TestMT(QString id, Module_Compass *compass, Module_IMU *adis, Module_PressureSensor *pressure)
    : RobotBehaviour_MT(id)
{
    this->compass = compass;
    this->adis = adis;
    this->pressure = pressure;
    setEnabled(false);

}

bool Behaviour_TestMT::isActive()
{
    return isEnabled();
}

void Behaviour_TestMT::start()
{
    this->setEnabled(true);
}



void Behaviour_TestMT::stop()
{

}

void Behaviour_TestMT::reset()
{

}

QList<RobotModule*> Behaviour_TestMT::getDependencies()
{
    QList<RobotModule*> ret;
    return ret;
}

QWidget* Behaviour_TestMT::createView(QWidget* parent)
{
    return new TestMTForm(parent, this);
}

