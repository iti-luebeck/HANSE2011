#include "behaviour_TestMT.h"
#include <QtGui>
#include <Behaviour_TestMT/testmtform.h>

Behaviour_TestMT::Behaviour_TestMT(QString id)
    : RobotBehaviour_MT(id)
{

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

