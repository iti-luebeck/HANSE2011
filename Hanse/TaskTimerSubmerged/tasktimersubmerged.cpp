#include "tasktimersubmerged.h"
#include "tasktimersubmergedform.h"
#include <QtGui>
#include <Module_Simulation/module_simulation.h>

TaskTimerSubmerged::TaskTimerSubmerged(QString id, Module_ThrusterControlLoop *tcl, Module_Simulation *sim)
    : RobotBehaviour(id)
{
    this->sim = sim;
    this->thrustercontrolloop = tcl;
}

bool TaskTimerSubmerged::isActive(){
    return active;
}


void TaskTimerSubmerged::init(){
    logger->debug("taskthrustercontrol init");

    active = false;

    // Default settings
    this->setDefaultValue("timerTime",1);
    this->setDefaultValue("targetDepth", 1.5);


    stopTimer.setSingleShot(true);
    stopTimer.moveToThread(this);

    connect(this,SIGNAL(setDepth(float)),thrustercontrolloop,SLOT(setDepth(float)));

    connect(&stopTimer, SIGNAL(timeout()), this, SLOT(stop()));
}

void TaskTimerSubmerged::startBehaviour(){
    if (isActive()){
        logger->info("Already active!");
        return;
    }

    logger->info("TaskTimerSubmerged started" );

    active = true;
    if(!isEnabled()){
        setEnabled(true);
    }
    this->reset();

    setHealthToOk();

    emit started(this);

    emit newStateOverview("TaskTimerSubmerged");

    if(!this->thrustercontrolloop->isEnabled()){
        this->thrustercontrolloop->setEnabled(true);
    }

    int time = this->getSettingsValue("timerTime").toInt()*60000;
    stopTimer.start(time);
    addData("submerged time:", this->getSettingsValue("timerTime").toString());
    emit setDepth(this->getSettingsValue("targetDepth").toFloat());
    addData("targetDepth:", this->getSettingsValue("targetDepth").toString());

    emit dataChanged(this);
    emit newState(this->getId(),"Submerging...");

}

void TaskTimerSubmerged::stop(){
    if(!isActive()){
        logger->info("Not active!");
        return;
    }

    logger->info("TaskTimerSubmerged stopped" );

    active = false;
    setEnabled(false);

    emit setDepth(0.0);
    emit newState(this->getId(),"Emerging successfully...");
    emit finished(this,true);

}


void TaskTimerSubmerged::emergencyStop()
{
    if (!isActive()){
        logger->info("Not active, no emergency stop needed");
        return;
    }
    stopTimer.stop();
    logger->info( "TaskTimerSubmerged emergency stopped" );

    active = false;
    setEnabled(false);

    emit setDepth(0.0);
    emit newState(this->getId(),"Emerging emergency...");
    emit finished(this,false);
}


void TaskTimerSubmerged::terminate()
{
    this->stop();
    RobotModule::terminate();
}

QWidget* TaskTimerSubmerged::createView(QWidget *parent)
{
    return new TaskTimerSubmergedForm(this, parent);
}

QList<RobotModule*> TaskTimerSubmerged::getDependencies()
{
    QList<RobotModule*> ret;
    ret.append(thrustercontrolloop);
    ret.append(sim);
    return ret;
}


void TaskTimerSubmerged::controlEnabledChanged(bool enabled){
    if(!enabled && isActive()){
        logger->info("Disable and deactivate TaskTimerSubmerged");
        stop();
    } else if(!enabled && !isActive()){
        //logger->info("Still deactivated");
    } else if(enabled && !isActive()){
        //logger->info("Enable and activate TaskTimerSubmerged");
        //startBehaviour();
    } else {
        //logger->info("Still activated");
    }
}
