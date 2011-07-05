#include "behaviour_pingerfollowing.h"
#include <QtGui>
#include <Behaviour_PingerFollowing/pingerfollowingform.h>
#include <Module_Simulation/module_simulation.h>
#include <Module_ThrusterControlLoop/module_thrustercontrolloop.h>
#include <Module_XsensMTi/module_xsensmti.h>
#include <Framework/Angles.h>

Behaviour_PingerFollowing::Behaviour_PingerFollowing(QString id, Module_ThrusterControlLoop* tcl, Module_Pinger *pinger, Module_XsensMTi* x)
    : RobotBehaviour(id)
{
    this->tcl = tcl;
    this->pinger = pinger;
    this->xsens = x;

}
bool Behaviour_PingerFollowing::isActive()
{
    return active;
}

void Behaviour_PingerFollowing::init()
{
    active = false;
    setEnabled(false);
    logger->debug("pinger init");


    connect(this,SIGNAL(forwardSpeed(float)),tcl,SLOT(setForwardSpeed(float)));
    connect(this,SIGNAL(angularSpeed(float)),tcl,SLOT(setAngularSpeed(float)));
    connect(this, SIGNAL(enabled(bool)), this, SLOT(controlEnabledChanged(bool)));
}

void Behaviour_PingerFollowing::startBehaviour()
{
    if (isActive()){
        logger->info("Already active!");
        return;
    }

    reset();

    logger->info(" - Behaviour started -");

    updateFromSettings();
    pinger->setEnabled(true);

    setHealthToOk();
    emit started(this);



    active = true;
    if(!isEnabled()){
        setEnabled(true);
    }

}


void Behaviour_PingerFollowing::stop()
{
    if(!isActive()){
        logger->info("Not active!");
        return;
    }

    active = false;
    setEnabled(false);

    emit forwardSpeed(0.0);
    emit angularSpeed(0.0);


    emit finished(this,false);

}

void Behaviour_PingerFollowing::terminate()
{
    stop();
    RobotModule::terminate();
}

void Behaviour_PingerFollowing::reset()
{
    emit forwardSpeed(0.0);
    emit angularSpeed(0.0);

}




void Behaviour_PingerFollowing::controlPingerFollow()
{
    if (!isActive()){
        return;
    }
}

void Behaviour_PingerFollowing::updateFromSettings()
{
    updateUi();


}

void Behaviour_PingerFollowing::controlEnabledChanged(bool enabled){
    if(!enabled && isActive()){
        logger->info("Disable and deactivate PingerFollowing");
        stop();
    } else if(!enabled && !isActive()){
        logger->info("Still deactivated");
    } else if(enabled && !isActive()){
        logger->info("Enable and activate PingerFollowing");
        startBehaviour();
    } else {
        logger->info("Still activated");
    }
}


QList<RobotModule*> Behaviour_PingerFollowing::getDependencies()
{
    QList<RobotModule*> ret;
    ret.append(tcl);
    ret.append(pinger);
    return ret;
}

QWidget* Behaviour_PingerFollowing::createView(QWidget* parent)
{
    return new PingerFollowingForm(parent, this);
}

