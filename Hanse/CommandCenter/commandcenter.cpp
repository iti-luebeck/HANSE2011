#include "commandcenter.h"
#include "commandcenterform.h"
#include <QtCore>
#include <stdio.h>
#include <Module_Simulation/module_simulation.h>
#include <CommandCenter/commandcenterform.h>
#include <Module_PressureSensor/module_pressuresensor.h>
#include <Module_ThrusterControlLoop/module_thrustercontrolloop.h>
#include <Module_HandControl/module_handcontrol.h>

CommandCenter::CommandCenter(QString id, Module_ThrusterControlLoop* tcl, Module_HandControl* handControl, Module_PressureSensor* pressure, Module_Simulation *sim, Behaviour_PipeFollowing* pipe, Behaviour_BallFollowing* ball, Behaviour_TurnOneEighty* o80, Behaviour_WallFollowing* wall, Behaviour_XsensFollowing* xsens, TaskHandControl *thc, TaskWallNavigation *twn)
    : RobotModule(id)
{
    qDebug()<<"commandcenter thread id";
    qDebug()<< QThread::currentThreadId();

    this->tcl = tcl;
    this->handControl = handControl;
    this->pressure = pressure;
    this->sim = sim;
    this->taskhandcontrol = thc;
    this->taskwallnavigation = twn;
    this->pipe = pipe;
    this->ball = ball;
    this->o80 = o80;
    this->wall = wall;
    this->xsens = xsens;
    //this->goal = goal;

    timer.moveToThread(this);

    setEnabled(false);
    running = false;

    // Command center specific signals
    connect(this,SIGNAL(setDepth(float)),tcl,SLOT(setDepth(float)));
    connect(this,SIGNAL(setForwardSpeed(float)),tcl,SLOT(setForwardSpeed(float)));
    connect(this,SIGNAL(setAngularSpeed(float)),tcl,SLOT(setAngularSpeed(float)));
    connect(this,SIGNAL(resetTCL()),tcl,SLOT(reset()));
    connect(handControl, SIGNAL(emergencyStop()), this, SLOT(emergencyStopCommandCenter()));
    connect(handControl, SIGNAL(startHandControl()), this, SLOT(startTaskHandControlCC()));
    connect(this, SIGNAL(taskTimeout()), this, SLOT(timeout()));
    connect(this, SIGNAL(cStop()), this, SLOT(stopCommandCenter()));

    // Stop all behaviours
    connect(handControl, SIGNAL(emergencyStop()), pipe, SLOT(stop()));
    //connect(handControl, SIGNAL(emergencyStop()), goal, SLOT(stop()));
    connect(handControl, SIGNAL(emergencyStop()), o80, SLOT(stop()));
    connect(handControl, SIGNAL(emergencyStop()), ball, SLOT(stop()));
    connect(handControl, SIGNAL(emergencyStop()), wall, SLOT(stop()));
    connect(handControl, SIGNAL(emergencyStop()), xsens, SLOT(stop()));

    // Tasks specific signals
    // TaskHandConrol
    connect(taskhandcontrol, SIGNAL(finished(RobotBehaviour*,bool)), this, SLOT(finishedControl(RobotBehaviour*,bool)));
    connect(this,SIGNAL(startTaskHandControl()),taskhandcontrol,SLOT(startBehaviour()));
    connect(this,SIGNAL(stopTaskHandControl()),taskhandcontrol,SLOT(stop()));
    connect(this,SIGNAL(stopAllTasks()),taskhandcontrol,SLOT(emergencyStop()));
    connect(taskhandcontrol,SIGNAL(handControlFinished()),this,SLOT(handControlFinishedCC()));


    // TaskWallNavigation
    connect(taskwallnavigation, SIGNAL(finished(RobotBehaviour*,bool)), this, SLOT(finishedControl(RobotBehaviour*,bool)));
    connect(this,SIGNAL(startTaskWallNavigation()),taskwallnavigation,SLOT(startBehaviour()));
    connect(this,SIGNAL(stopTaskWallNavigation()),taskwallnavigation,SLOT(stop()));
    connect(this,SIGNAL(stopAllTasks()),taskwallnavigation,SLOT(emergencyStop()));

    setDefaultValue("targetDepth",0.42);
    setDefaultValue("subEx", false);
    setDefaultValue("waitTime",2000);

    count = 1;

    controlTimer.moveToThread(this);
}


bool CommandCenter::isActive(){
    return isEnabled();
}

void CommandCenter::init(){
    logger->debug("Command center init");
    this->setEnabled(true);
}

void CommandCenter::startCommandCenter(){
    RobotModule::reset();
    running = true;

    emit newMessage("CommandCenter started!");

    this->abortedList.clear();
    this->finishedList.clear();
    emit updateGUI();


    // No Handcontrol, it is commandcenter time!
    if(this->handControl->isEnabled()){
        this->handControl->setEnabled(false);
    }

    // Every task needs thruster
    if(!this->tcl->isEnabled()){
        this->tcl->setEnabled(true);
    }

    logger->info("CommandCenter started");

    if(this->getSettingsValue("subEx").toBool() == true){
        logger->debug("Submerged execution!");
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
    //this->setHealthToSick("Commandcenter stoped, everything stop/disable");
    //this->setEnabled(false);
    controlTimer.stop();

    scheduleList.clear();
    emit stopAllTasks();
    emit resetTCL();

    emit setDepth(0.0);
    emit setForwardSpeed(0.0);
    emit setAngularSpeed(0.0);


    emit updateGUI();

    if (sim->isEnabled())
    {

    }
}

void CommandCenter::commandCenterControl(){

    if(!this->tcl->isEnabled()){
        this->tcl->setEnabled(true);
    }

    if(this->handControl->isEnabled()){
        this->handControl->setEnabled(false);
    }
    if(!scheduleList.isEmpty() && this->isEnabled()){
        logger->debug("Commandcenter control; Next scheduled task:");
        QString temp=scheduleList.first();
        QString tempAkt = "";
        logger->debug(temp);
        for(int j = 0; j < temp.length(); j++){
            if(((temp[j] == QChar(':'))==false) && ((temp[j] == QChar(' '))==false) ){
                tempAkt = tempAkt + temp.at(j);
            } else {
                break;
            }
        }
        logger->debug(tempAkt);
        scheduleList.removeFirst();

        if(tempAkt == "HandControl"){
            emit startTaskHandControl();
            activeTask = tempAkt;
        } else if(tempAkt == "TaskWallNavigation"){
            emit startTaskWallNavigation();
            activeTask = tempAkt;
        } else  {
            logger->info("Task not found, skip task!");
            emit newError("Task not found, skip task!");
            this->abortedList.append(temp);
            activeTask = "";
            commandCenterControl();
        }
        count++;
    } else {
        if(!this->isEnabled()){
            emit newError("Commandcenter not enabled!");
        } else {
            logger->info("Schedule error, no existing tasks!");
            if(!this->getSettingsValue("subEx").toBool()){
                emit newError("No existing task");
            } else {
                emit newError("No existing task, surface");
                emit setDepth(0.0);
                emit setForwardSpeed(0.0);
                emit setAngularSpeed(0.0);
            }
        }
    }
    emit updateGUI();
}

void CommandCenter::finishedControl(RobotBehaviour *, bool success){
    // Evaluate task success


    if(!this->tcl->isEnabled() && !this->handControl->isEnabled()){
        this->handControl->setEnabled(true);
        this->tcl->setEnabled(true);
    }

    emit setAngularSpeed(0.0);
    emit setForwardSpeed(0.0);
    if(this->isEnabled()){
        if(this->getSettingsValue("subEx").toBool() == true){
            emit setDepth(this->getSettingsValue("targetDepth").toFloat());
        } else {
            emit setDepth(0.0);
        }
    }
    if(success==true){
        logger->info("One task has finished successfully");
        this->finishedList.append(activeTask);
        activeTask = "";
    } else {
        logger->info("One task has finished unsuccessfully");
        this->abortedList.append(activeTask);
        activeTask = "";
    }
    if(this->isEnabled()){
        // Stop thruster, then make next task...
        controlTimer.singleShot(this->getSettingsValue("waitTime").toInt(),this, SLOT(doNextTask()));
    }

    // Update finished/aborted list
    emit updateGUI();
}


void CommandCenter::doNextTask(){
    // After a wait time, the next task will be executed
    commandCenterControl();
}

// Not used....
void CommandCenter::timeout(){
    // Timeout, stop everything
    logger->info("Commandcenter timeout!");
    emit setDepth(0);
    emit setForwardSpeed(0);
    emit setAngularSpeed(0);
    stopCommandCenter();
}

void CommandCenter::submergedExecute(){
    // Set submerged depth
    logger->debug("Commandcenter submerged execute!");
    addData("Sub.exec.depth", this->getSettingsValue("targetDepth").toFloat());
    emit dataChanged(this);
    emit setDepth(this->getSettingsValue("targetDepth").toFloat());
}


void CommandCenter::terminate(){
    RobotModule::terminate();
    emit stopAllTasks();
    this->stopCommandCenter();
}

void CommandCenter::reset(){
    RobotModule::reset();
    emit newError("Reset");

    emit stopAllTasks();

    this->scheduleList.clear();
    this->finishedList.clear();
    this->abortedList.clear();

    emit updateGUI();
}

void CommandCenter::setNewMessage(QString s){
    emit newMessage(s);
}


QList<RobotModule*> CommandCenter::getDependencies(){
    QList<RobotModule*> ret;
    ret.append(sim);
    return ret;
}

QWidget* CommandCenter::createView(QWidget *parent){
    return new CommandCenterForm(this, parent);
}

void CommandCenter::emergencyStopCommandCenter(){
    controlTimer.stop();
    logger ->info("Emergency stop, stop and deactivate all task - handcontrol active");
    emit stopAllTasks();
    emit resetTCL();

    this->handControl->setEnabled(false);
    emit setDepth(0.0);
    emit setForwardSpeed(0.0);
    emit setAngularSpeed(0.0);
    emit updateGUI();
}

void CommandCenter::startTaskHandControlCC(){
    controlTimer.stop();
    logger ->info("Stop and deactivate all task - handcontrol active");
    emit stopAllTasks();
    emit resetTCL();
    if(this->activeTask != "HandControl"){
        emit startTaskHandControl();
        activeTask = "HandControl";
    }
    emit setDepth(0.0);
    emit setForwardSpeed(0.0);
    emit setAngularSpeed(0.0);
    emit updateGUI();
}

void CommandCenter::handControlFinishedCC(){
    emit setAngularSpeed(0.0);
    emit setForwardSpeed(0.0);
    if(this->getSettingsValue("subEx").toBool() == true){
        emit setDepth(this->getSettingsValue("targetDepth").toFloat());
    } else {
        emit setDepth(0.0);
    }

    logger->info("Handcontrol has finished successfully");
    this->finishedList.append("HandControl");

    // Stop thruster, then make next task...
    controlTimer.singleShot(this->getSettingsValue("waitTime").toInt(),this, SLOT(doNextTask()));

    activeTask = "";
    emit updateGUI();
}

void CommandCenter::addTask(QString listName, QString taskName){
    if(listName == "scheduleList"){
        this->scheduleList.append(taskName);
    } else if(listName == "abortedList"){
        this->abortedList.append(taskName);
    } else if(listName == "finishedList"){
        this->finishedList.append(taskName);
    } else {
        logger->info("List not found!");
    }
    updateGUI();
}

void CommandCenter::clearList(QString listName){
    if(listName == "scheduleList"){
        this->scheduleList.clear();
    } else if(listName == "abortedList"){
        this->abortedList.clear();
    } else if(listName == "finishedList"){
        this->finishedList.clear();
    } else {
        logger->info("List not found!");
    }
    updateGUI();
}

void CommandCenter::removeTask(){
    if(!this->scheduleList.isEmpty()){
        this->scheduleList.removeLast();
    }
    emit updateGUI();
}
