#include "behaviour_wallfollowing.h"
#include <QtGui>
#include <Behaviour_WallFollowing/wallfollowingform.h>

Behaviour_WallFollowing::Behaviour_WallFollowing(QString id, Module_EchoSounder *echo)
    : RobotBehaviour(id)
{
    this->echo = echo;
    setEnabled(false);

}

bool Behaviour_WallFollowing::isActive()
{
    return isEnabled();
}

void Behaviour_WallFollowing::startBehaviour()
{
    this->setEnabled(true);
}



void Behaviour_WallFollowing::stop()
{

}

void Behaviour_WallFollowing::reset()
{

}

QList<RobotModule*> Behaviour_WallFollowing::getDependencies()
{
    QList<RobotModule*> ret;
    return ret;
}

QWidget* Behaviour_WallFollowing::createView(QWidget* parent)
{
    return new WallFollowingForm(parent, this);
}

