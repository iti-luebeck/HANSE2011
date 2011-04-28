#include "commandcenter.h"
#include "commandcenterform.h"
#include <QtCore>
#include <stdio.h>
#include <Module_Simulation/module_simulation.h>
#include <CommandCenter/commandcenterform.h>
#include <Module_PressureSensor/module_pressuresensor.h>
#include <Module_ThrusterControlLoop/module_thrustercontrolloop.h>
#include <Module_HandControl/module_handcontrol.h>

CommandCenter::CommandCenter(QString id, Module_ThrusterControlLoop* tcl, Module_HandControl* handControl, Module_PressureSensor* pressure, Module_Simulation *sim, TestTask *tt, TaskWallFollowing *twf)
    : RobotModule(id)
{
    qDebug()<<"commandcenter thread id";
    qDebug()<< QThread::currentThreadId();

    this->tcl = tcl;
    this->handControl = handControl;
    this->pressure = pressure;
    this->sim = sim;
    this->testtask = tt;
    this->taskwallfollowing = twf;


    timer.moveToThread(this);

    setEnabled(false);
    running = false;



    // Command center specific signals
    connect(this,SIGNAL(setDepth(float)),tcl,SLOT(setDepth(float)));
    connect(this,SIGNAL(setForwardSpeed(float)),tcl,SLOT(setForwardSpeed(float)));
    connect(this,SIGNAL(setAngularSpeed(float)),tcl,SLOT(setAngularSpeed(float)));
    connect(this,SIGNAL(resetTCL()),tcl,SLOT(reset()));
    connect(handControl, SIGNAL(emergencyStop()), this, SLOT(emergencyStop()));
    connect(handControl, SIGNAL(startHandControl()), this, SLOT(startHandControl()));
    connect(this, SIGNAL(taskTimeout()), this, SLOT(timeout()));
    connect(this, SIGNAL(cStop()), this, SLOT(stopCommandCenter()));

    // Tasks specific signals
    connect(testtask, SIGNAL(finished(RobotBehaviour*,bool)), this, SLOT(finishedControl(RobotBehaviour*,bool)));
    connect(this,SIGNAL(startTestTask()),testtask,SLOT(startBehaviour()));
    connect(this,SIGNAL(stopTestTask()),testtask,SLOT(stop()));
    connect(this,SIGNAL(stopAllTasks()),testtask,SLOT(emergencyStop()));


    connect(taskwallfollowing, SIGNAL(finished(RobotBehaviour*,bool)), this, SLOT(finishedControl(RobotBehaviour*,bool)));
    connect(this,SIGNAL(startTaskWallFollowing()),taskwallfollowing,SLOT(startBehaviour()));
    connect(this,SIGNAL(stopTaskWallFollowing()),taskwallfollowing,SLOT(stop()));
    connect(this,SIGNAL(stopAllTasks()),taskwallfollowing,SLOT(emergencyStop()));


    setDefaultValue("targetDepth",0.30);
    setDefaultValue("subEx", false);
    setDefaultValue("waitTime",5000);

    count = 1;
    countTaskWall = 0;

    controlTimer.moveToThread(this);
}


bool CommandCenter::isActive(){
    return isEnabled();
}

void CommandCenter::init(){
    logger->debug("Command center init");
}

void CommandCenter::startCommandCenter(){
    RobotModule::reset();
    running = true;
    this->setEnabled(true);
    logger->info("CommandCenter started");

    if(this->getSettingsValue("subEx").toBool() == true){
        // qDebug("Submerged execution!");
        addData("Submerged",true);
        emit dataChanged(this);
        submergedExecute();
    } else {
        addData("Submerged",false);
        emit dataChanged(this);
    }
    addData("Waittime", this->getSettingsValue("waitTime"));
    emit dataChanged(this);
    commandCenterControl();
}

void CommandCenter::stopCommandCenter(){
    RobotModule::reset();

    logger->info("CommandCenter stoped");
    this->setHealthToSick("Commandcenter stoped");
    this->setEnabled(false);

    schedule.clear();
    emit stopAllTasks();
    emit resetTCL();

    emit setDepth(0);
    emit setForwardSpeed(0);
    emit setAngularSpeed(0);

    if (sim->isEnabled())
    {

    }
}

void CommandCenter::commandCenterControl(){
    if(!schedule.isEmpty() & this->isEnabled()){
        qDebug("Commandcenter control; Next scheduled task:");
        QString temp=schedule.last();
        qDebug()<<temp;

        schedule.removeLast();
        addData("Task Nr", count);
        addData("Task Name", temp);
        emit dataChanged(this);
        if(temp == "TestTask"){
            emit startTestTask();
            emit newList("");
            emit currentTask(temp);
            lTask = temp;
        } else if(temp == "TaskWallFollowing"){
            // Sets specific settings for each drive
            if(countTaskWall==0){
                // No setting change, use GUI input
            } else if(countTaskWall==1){
                changeWallTaskSettings(0.5, 0.3, 1.0, 0.1, 10000);
            } else if (countTaskWall==2){
                changeWallTaskSettings(0.5, 0.3, 1.5, 0.2, 30000);
            } else {
                changeWallTaskSettings(0.5, 0.3, 1.5, 0.1, 15000);
            }
            emit startTaskWallFollowing();
            emit newList("");
            emit currentTask(temp);
            lTask = temp;
            countTaskWall = countTaskWall+1;
        } else {
            qDebug("Task not found, skip task!");
            emit newError("Task not found, skip task!");
            emit newAborted(temp);
            commandCenterControl();
        }
        count++;
    } else {
        qDebug("Schedule error, no existing tasks!");
        emit newError("No existing task");
        cStop();
    }


}

void CommandCenter::changeWallTaskSettings(float fwS, float agS, float desD, float crW, int taskD){
    qDebug("Change taskwallfollowing settings!");
    this->taskwallfollowing->setSettingsValue("forwardSpeed", fwS);
    this->taskwallfollowing->setSettingsValue("angularSpeed", agS);
    this->taskwallfollowing->setSettingsValue("desiredDistance", desD);
    this->taskwallfollowing->setSettingsValue("corridorWidth", crW);
    this->taskwallfollowing->setSettingsValue("taskDuration", taskD);
}

void CommandCenter::finishedControl(RobotBehaviour *, bool success){
    // Evaluate task success
    if(success==true){
        qDebug("One task has finished successfully");
        emit newList(lTask);
    } else {
        qDebug("One task has finished unsuccessfully");
        emit newAborted(lTask);
        emit newList("");
    }
    if(this->isEnabled()){
        controlTimer.singleShot(this->getSettingsValue("waitTime").toInt(),this, SLOT(doNextTask()));
    }
}


void CommandCenter::doNextTask(){
    // After a wait time, the next task will be executed
    commandCenterControl();
}

void CommandCenter::timeout(){
    // Timeout, stop everything
    qDebug("Commandcenter timeout!");
    emit setDepth(0);
    emit setForwardSpeed(0);
    emit setAngularSpeed(0);
    stopCommandCenter();
}

void CommandCenter::submergedExecute(){
    // Set submerged depth
    qDebug("Commandcenter submerged execute!");
    addData("Sub.exec.depth", this->getSettingsValue("targetDepth").toFloat());
    emit dataChanged(this);
    emit setDepth(this->getSettingsValue("targetDepth").toFloat());
}


void CommandCenter::terminate(){
    RobotModule::terminate();
    this->stopAllTasks();
    this->stopCommandCenter();
}

void CommandCenter::reset(){
    RobotModule::reset();
    // There is nothing to reset...
}

QList<RobotModule*> CommandCenter::getDependencies(){
    QList<RobotModule*> ret;
    ret.append(sim);
    return ret;
}

QWidget* CommandCenter::createView(QWidget *parent){
    return new CommandCenterForm(this, parent);
}
