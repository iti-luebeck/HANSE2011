#include "behaviour_groundfollowing.h"

#include <Module_ThrusterControlLoop/module_thrustercontrolloop.h>
#include <QtGui>
#include <Behaviour_GroundFollowing/groundfollowingform.h>
#include <Module_Simulation/module_simulation.h>

Behaviour_GroundFollowing::Behaviour_GroundFollowing(QString id, Module_ThrusterControlLoop *tcl, Module_EchoSounder *echo, Module_Simulation *sim)
    : RobotBehaviour(id)
{

    qDebug() << "ground thread id";
    qDebug() << QThread::currentThreadId();
    this->tcl = tcl;
    this->echo = echo;
    this->sim = sim;

    setEnabled(false);
}


bool Behaviour_GroundFollowing::isActive()
{
    return isEnabled();
}

void Behaviour_GroundFollowing::init()
{
    logger->debug("ground init");
    timer.moveToThread(this);

}

void Behaviour_GroundFollowing::startBehaviour()
{

}

void Behaviour_GroundFollowing::stop()
{
}

void Behaviour_GroundFollowing::terminate()
{
    this->stop();
    RobotModule::terminate();

}

void Behaviour_GroundFollowing::reset()
{
    RobotBehaviour::reset();
}

QWidget* Behaviour_GroundFollowing::createView(QWidget* parent)
{
    return new GroundFollowingForm( parent, this);
}

QList<RobotModule*> Behaviour_GroundFollowing::getDependencies()
{
    QList<RobotModule*> ret;
    ret.append(tcl);
    ret.append(echo);
    ret.append(sim);
    return ret;
}


