#include "taskhandcontrol.h"
#include "taskhandcontrolform.h"
#include <QtGui>
#include <Module_Simulation/module_simulation.h>

TaskHandControl::TaskHandControl(QString id, Module_ThrusterControlLoop *tcl, Module_Simulation *sim, Module_HandControl *hc)
    : RobotBehaviour(id)
{
    this->sim = sim;
    this->thrustercontrolloop = tcl;
    this->handcontrol = hc;

}

bool TaskHandControl::isActive(){
    return active;
}


void TaskHandControl::init(){
    logger->debug("taskhandcontrol init");

    active = false;
    setEnabled(false);

    testTimer.setSingleShot(true);
    testTimer.moveToThread(this);


    connect(this,SIGNAL(forwardSpeed(float)),thrustercontrolloop,SLOT(setForwardSpeed(float)));

    connect(this,SIGNAL(angularSpeed(float)),thrustercontrolloop,SLOT(setAngularSpeed(float)));

    connect(this,SIGNAL(setDepth(float)),thrustercontrolloop,SLOT(setDepth(float)));
}

void TaskHandControl::startBehaviour(){
    if (isActive()){
        logger->info("Already active!");
        return;
    }

    logger->info("TaskHandControl started" );

    active = true;
    if(!isEnabled()){
        setEnabled(true);
    }
    this->reset();

    setHealthToOk();

    emit started(this);

    emit newStateOverview("Handcontrol");

    if(!this->handcontrol->isEnabled()){
        this->handcontrol->setEnabled(true);
    }

    if(!this->thrustercontrolloop->isEnabled()){
        this->thrustercontrolloop->setEnabled(true);
    }
    emit newState("HandControl","HandControl");
}

void TaskHandControl::stop(){
    if(!isActive()){
        logger->info("Not active!");
        return;
    }

    logger->info( "Task handcontrol stopped" );

    active = false;
    setEnabled(false);

    emit forwardSpeed(0.0);
    emit angularSpeed(0.0);

    this->handcontrol->setEnabled(false);
    emit finished(this,true);

}


void TaskHandControl::emergencyStop()
{
    if (!isActive()){
        logger->info("Not active, no emergency stop needed");
        return;
    }

    logger->info( "Task handcontrol emergency stopped" );

    active = false;
    setEnabled(false);

    this->handcontrol->setEnabled(false);
    emit forwardSpeed(0.0);
    emit angularSpeed(0.0);
    emit setDepth(0.0);
    emit finished(this,false);

}


void TaskHandControl::terminate()
{
    this->stop();
    RobotModule::terminate();
}

QWidget* TaskHandControl::createView(QWidget *parent)
{
    return new TaskHandControlForm(this, parent);
}

QList<RobotModule*> TaskHandControl::getDependencies()
{
    QList<RobotModule*> ret;
    ret.append(thrustercontrolloop);
    ret.append(sim);
    return ret;
}

void TaskHandControl::handControlFinishedSlot(){
    if(!isActive()){
        return;
    }

    emit forwardSpeed(0.0);
    emit angularSpeed(0.0);
    this->handcontrol->setEnabled(false);
    active = false;
    setEnabled(false);
    emit finished(this, true);
}


void TaskHandControl::controlEnabledChanged(bool enabled){
    if(!enabled && isActive()){
        logger->info("Disable and deactivate TaskHandControl");
        stop();
    } else if(!enabled && !isActive()){
        //logger->info("Still deactivated");
    } else if(enabled && !isActive()){
        //logger->info("Enable and activate TaskHandControl");
        //startBehaviour();
    } else {
        //logger->info("Still activated");
    }
}
