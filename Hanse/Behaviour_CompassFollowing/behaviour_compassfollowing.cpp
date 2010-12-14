#include "behaviour_compassfollowing.h"
#include <QtGui>
#include <Behaviour_CompassFollowing/compassfollowingform.h>

Behaviour_CompassFollowing::Behaviour_CompassFollowing(QString id, Module_ThrusterControlLoop *tcl, Module_Compass *compass)
    : RobotBehaviour_MT(id)
{
    this->compass = compass;
    this->tcl = tcl;
    setEnabled(false);

}

bool Behaviour_CompassFollowing::isActive()
{
    return isEnabled();
}

void Behaviour_CompassFollowing::start()
{
    this->setEnabled(true);
    timer.start(100);
}

void Behaviour_CompassFollowing::stop()
{
    this->setEnabled(false);
}

void Behaviour_CompassFollowing::reset()
{

}

void Behaviour_CompassFollowing::controlLoop()
{

}

QList<RobotModule*> Behaviour_CompassFollowing::getDependencies()
{
    QList<RobotModule*> ret;
    ret.append(tcl);
    ret.append(compass);
    return ret;
}

QWidget* Behaviour_CompassFollowing::createView(QWidget* parent)
{
    return new CompassFollowingForm(parent, this);
}

