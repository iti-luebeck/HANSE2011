#include "taskhandcontrol.h"
#include "taskhandcontrolform.h"
#include <QtGui>
#include <Module_Simulation/module_simulation.h>

TaskHandControl::TaskHandControl(QString id, Module_ThrusterControlLoop *tcl, Module_Simulation *sim, Module_HandControl *hc)
    : RobotBehaviour(id)
{
    qDebug()<<"taskthrustercontrol thread id";
    qDebug()<< QThread::currentThreadId();
    this->sim = sim;
    this->thrustercontrolloop = tcl;
    this->handcontrol = hc;

    setEnabled(false);
    running = false;

    // Default settings
    this->setDefaultValue("forwardSpeed1",0.5);
    this->setDefaultValue("angularSpeed1",0.0);
    this->setDefaultValue("desiredDepth1",0.0);
    this->setDefaultValue("taskDuration1",10000);
    this->setDefaultValue("description1", "task 1");


    testTimer.setSingleShot(true);
    testTimer.moveToThread(this);


    connect(this,SIGNAL(forwardSpeed(float)),thrustercontrolloop,SLOT(setForwardSpeed(float)));

    connect(this,SIGNAL(angularSpeed(float)),thrustercontrolloop,SLOT(setAngularSpeed(float)));

    connect(this,SIGNAL(setDepth(float)),thrustercontrolloop,SLOT(setDepth(float)));
}

bool TaskHandControl::isActive(){
    return isEnabled();
}


void TaskHandControl::init(){
    logger->debug("taskthrustercontrol init");
}



void TaskHandControl::startBehaviour(){
    this->reset();
    logger->info("TaskHandControl started" );
    running = true;
    setHealthToOk();
    setEnabled(true);
    emit started(this);
    running = true;

    if(!this->handcontrol->isEnabled()){
        this->handcontrol->setEnabled(true);
    }

    if(!this->thrustercontrolloop->isEnabled()){
        this->thrustercontrolloop->setEnabled(true);
    }
}

void TaskHandControl::stop(){
    running = false;
    logger->info( "Task handcontrol stopped" );

    if (this->isActive())
    {


        emit forwardSpeed(0.0);
        emit angularSpeed(0.0);

        this->setEnabled(false);
        this->handcontrol->setEnabled(false);
        emit finished(this,true);
    }
}


void TaskHandControl::emergencyStop()
{
    running = false;
    logger->info( "Task handcontrol emergency stopped" );

    if (this->isActive())
    {
        this->handcontrol->setEnabled(false);
        emit forwardSpeed(0.0);
        emit angularSpeed(0.0);
        emit setDepth(0.0);
        this->setEnabled(false);
        emit finished(this,false);
    }
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
    emit forwardSpeed(0.0);
    emit angularSpeed(0.0);
    this->handcontrol->setEnabled(false);
    this->setEnabled(false);
    emit handControlFinished();
}
