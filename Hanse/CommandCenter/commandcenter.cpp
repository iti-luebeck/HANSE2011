#include "commandcenter.h"
#include "commandcenterform.h"
#include <QtCore>
#include <stdio.h>
#include <Module_Simulation/module_simulation.h>
#include <CommandCenter/commandcenterform.h>
#include <Module_PressureSensor/module_pressuresensor.h>
#include <Module_ThrusterControlLoop/module_thrustercontrolloop.h>
#include <Module_HandControl/module_handcontrol.h>
#include <TaskHandControl/taskhandcontrol.h>
#include <TaskWallNavigation/taskwallnavigation.h>
#include <Behaviour_PipeFollowing/behaviour_pipefollowing.h>
#include <Behaviour_WallFollowing/behaviour_wallfollowing.h>
#include <Behaviour_TurnOneEighty/behaviour_turnoneeighty.h>
#include <Behaviour_PipeFollowing/behaviour_pipefollowing.h>
#include <Behaviour_BallFollowing/behaviour_ballfollowing.h>
//#include <Behaviour_GoalFollowing/behaviour_goalfollowing.h>
#include <Behaviour_XsensFollowing/behaviour_xsensfollowing.h>
#include <TaskXsensNavigation/taskxsensnavigation.h>
#include <Module_Navigation/module_navigation.h>
#include <TaskPipeFollowing/taskpipefollowing.h>

CommandCenter::CommandCenter(QString id, Module_ThrusterControlLoop* tcl, Module_HandControl* handControl, Module_PressureSensor* pressure, Module_Simulation *sim, Module_Navigation *n, Behaviour_PipeFollowing* pipe, Behaviour_BallFollowing* ball, Behaviour_TurnOneEighty* o80, Behaviour_WallFollowing* wall, Behaviour_XsensFollowing* xsens, TaskHandControl *thc, TaskWallNavigation *twn,TaskXsensNavigation *txn, TaskPipeFollowing *tpf)
    : RobotModule(id)
{
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
    this->taskxsensnavigation = txn;
    this->navi = n;
    //this->goal = goal;
    this->taskpipefollowing = tpf;

    this->behaviour.append(pipe);
    this->behaviour.append(ball);
    this->behaviour.append(o80);
    this->behaviour.append(wall);
    this->behaviour.append(xsens);

    // Emergency stop all behaviours
    foreach (RobotBehaviour* b, behaviour)
    {
        connect(handControl, SIGNAL(stopAllTasks()), b, SLOT(stop()));
    }

    // Tasks specific signals
    this->taskList.append(taskwallnavigation);
    this->taskList.append(taskxsensnavigation);
    this->taskList.append(taskpipefollowing);

    foreach (RobotBehaviour* b, taskList)
    {
        connect(b, SIGNAL(finished(RobotBehaviour*,bool)), this, SLOT(finishedControl(RobotBehaviour*,bool)));
        connect(this,SIGNAL(stopAllTasks()),b,SLOT(emergencyStop()));
        connect(b, SIGNAL(newState(QString)), this, SLOT(updateState(QString)));
        connect(b, SIGNAL(newStateOverview(QString)), this, SLOT(updateStateOverview(QString)));
    }

    // Task WallNavigation
    connect(this,SIGNAL(startTaskWallNavigation()),taskwallnavigation,SLOT(startBehaviour()));
    connect(this,SIGNAL(stopTaskWallNavigation()),taskwallnavigation,SLOT(stop()));

    // Task XsensNavigation
    connect(this,SIGNAL(startTaskXsensNavigation()),taskxsensnavigation,SLOT(startBehaviour()));
    connect(this,SIGNAL(stopTaskXsensNavigation()),taskxsensnavigation,SLOT(stop()));


    // Task PipeFollowing
    connect(this, SIGNAL(startTaskPipeFollowing()), taskpipefollowing, SLOT(startBehaviour()));
    connect(this, SIGNAL(stopTaskPipeFollowing()), taskpipefollowing, SLOT(stop()));

    // Task HandControl
    this->taskList.append(taskhandcontrol);
    connect(taskhandcontrol, SIGNAL(finished(RobotBehaviour*,bool)), this, SLOT(handControlFinishedCC(RobotBehaviour*,bool)));
    connect(this,SIGNAL(startTaskHandControl()),taskhandcontrol,SLOT(startBehaviour()));
    connect(this,SIGNAL(stopTaskHandControl()),taskhandcontrol,SLOT(stop()));
    connect(this,SIGNAL(stopAllTasks()),taskhandcontrol,SLOT(emergencyStop()));
    connect(taskhandcontrol, SIGNAL(newState(QString)), this, SLOT(updateState(QString)));

    // Add task to GUI list
    for(int i = 0; i < this->taskList.length(); i++){
        this->taskInputList.append(this->taskList.at(i)->getTabName());
    }

    // Command center specific signals
    connect(this,SIGNAL(setDepth(float)),tcl,SLOT(setDepth(float)));
    connect(this,SIGNAL(setForwardSpeed(float)),tcl,SLOT(setForwardSpeed(float)));
    connect(this,SIGNAL(setAngularSpeed(float)),tcl,SLOT(setAngularSpeed(float)));
    connect(this,SIGNAL(resetTCL()),tcl,SLOT(reset()));

    connect(handControl, SIGNAL(emergencyStop()), this, SLOT(emergencyStopCommandCenter()));
    connect(handControl, SIGNAL(startHandControl()), this, SLOT(startTaskHandControlCC()));
    connect(handControl, SIGNAL(enabled(bool)), this, SLOT(controlTaskHandControl(bool)));

    connect(this, SIGNAL(taskTimeout()), this, SLOT(timeout()));
    connect(this, SIGNAL(cStop()), this, SLOT(stopCommandCenter()));

    // Default values
    setDefaultValue("targetDepth",0.42);
    setDefaultValue("subEx", false);
    setDefaultValue("waitTime",2000);

    controlTimer.moveToThread(this);
    timer.moveToThread(this);

    setEnabled(false);
    running = false;
}


bool CommandCenter::isActive(){
    return isEnabled();
}

void CommandCenter::init(){
    logger->debug("Commandcenter init");
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
    if (this->isEnabled() == false){
        logger->info("CC not enabled!");
        return;
    }
    running = false;

    RobotModule::reset();
    logger->info("CommandCenter stoped");
    //this->setHealthToSick("Commandcenter stoped, everything stop/disable");
    //this->setEnabled(false);

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
    if (this->isEnabled() == false){
        logger->info("CC not enabled!");
        return;
    }

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

        if(tempAkt == "taskHand"){
            emit startTaskHandControl();
            activeTask = tempAkt;
        } else if(tempAkt == "taskWallNavi"){
            emit startTaskWallNavigation();
            activeTask = tempAkt;
        } else if(tempAkt == "taskXsensNavi"){
            emit startTaskXsensNavigation();
            activeTask = tempAkt;
        } else if(tempAkt == "taskPipeFollow"){
            emit startTaskPipeFollowing();
            activeTask = tempAkt;
        } else  {
            logger->info("Task not found, skip task!");
            emit newError("Task not found, skip task!");
            this->abortedList.append(temp);
            activeTask = "";
            commandCenterControl();
        }
    } else {
        if(!this->isEnabled()){
            emit newError("Commandcenter not enabled!");
        } else {
            logger->info("Schedule error, no existing tasks!");
            if(!this->getSettingsValue("subEx").toBool()){
                emit newError("No existing task");
            } else {
                emit newError("No existing task, surface");
            }
            emit setDepth(0.0);
            emit setForwardSpeed(0.0);
            emit setAngularSpeed(0.0);
        }
    }
    emit updateGUI();
}

void CommandCenter::finishedControl(RobotBehaviour *name, bool success){
    // Evaluate task success
    if (this->isEnabled() == false){
        logger->info("CC not enabled!");
        return;
    }

    if(!this->tcl->isEnabled() && !this->handControl->isEnabled()){
        this->handControl->setEnabled(true);
        this->tcl->setEnabled(true);
    }

    emit newState("");
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
        this->finishedList.append(name->getTabName());
        activeTask = "";
    } else {
        logger->info("One task has finished unsuccessfully");
        this->abortedList.append(name->getTabName());
        activeTask = "";
    }
    if(this->isEnabled()){
        // Stop thruster, then make next task...
        if(this->taskhandcontrol->isEnabled()  == false && running == true){
            controlTimer.singleShot(this->getSettingsValue("waitTime").toInt(),this, SLOT(doNextTask()));
        }
    }
    // Update finished/aborted list
    emit updateGUI();
    emit newStateOverview("CLEAR");
}


void CommandCenter::doNextTask(){
    if (this->isEnabled() == false){
        logger->info("CC not enabled!");
        return;
    }
    // After a wait time, the next task will be executed
    commandCenterControl();
}

// Not used....
void CommandCenter::timeout(){
    if (this->isEnabled() == false){
        logger->info("CC not enabled!");
        return;
    }
    // Timeout, stop everything
    logger->info("Commandcenter timeout!");
    emit setDepth(0);
    emit setForwardSpeed(0);
    emit setAngularSpeed(0);
    stopCommandCenter();
    running = false;
}

void CommandCenter::submergedExecute(){
    if (this->isEnabled() == false){
        logger->info("CC not enabled!");
        return;
    }
    // Set submerged depth
    logger->debug("Commandcenter submerged execute!");
    addData("Sub.exec.depth", this->getSettingsValue("targetDepth").toFloat());
    emit dataChanged(this);
    emit setDepth(this->getSettingsValue("targetDepth").toFloat());
}


void CommandCenter::terminate(){
    RobotModule::terminate();
    logger->info("Terminate, stop all tasks");
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
    if (this->isEnabled() == false){
        logger->info("CC not enabled!");
        return;
    }
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
    if (this->isEnabled() == false){
        logger->info("CC not enabled!");
        return;
    }
    running = false;
    logger ->info("Emergency stop, stop and deactivate all task - handcontrol active");
    emit stopAllTasks();
    emit resetTCL();

    emit setDepth(0.0);
    emit setForwardSpeed(0.0);
    emit setAngularSpeed(0.0);
    emit updateGUI();

}


void CommandCenter::addTask(QString listName, QString taskName){
    if (this->isEnabled() == false){
        logger->info("Not enabled!");
        return;
    }
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
    if (this->isEnabled() == false){
        logger->info("Not enabled!");
        return;
    }
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
    if (this->isEnabled() == false){
        logger->info("Not enabled!");
        return;
    }
    if(!this->scheduleList.isEmpty()){
        this->scheduleList.removeLast();
    }
    emit updateGUI();
}

void CommandCenter::skipTask(){
    if(!this->scheduleList.isEmpty()){
        this->abortedList.append(this->scheduleList.takeFirst());
    }
    emit updateGUI();
}

void CommandCenter::updateState(QString state){
    if (this->isEnabled() == false){
        logger->info("Not enabled!");
        return;
    }
    emit newState(state);
}

void CommandCenter::updateStateOverview(QString state){
    if (this->isEnabled() == false){
        logger->info("Not enabled!");
        return;
    }
    emit newStateOverview(state);
}

void CommandCenter::controlTaskHandControl(bool b){
    if (this->isEnabled() == false){
        logger->info("Not enabled!");
        return;
    }
    this->wait(100);
    if(b == true){
        if (this->taskhandcontrol->isEnabled() == true){
            logger->info("TaskHandcontrol already enabled!");
            return;
        } else {
            QTimer::singleShot(0, this, SLOT(startTaskHandControlCC()));
        }
    } else {
        if (this->taskhandcontrol->isEnabled() == false){
            logger->info("TaskHandcontrol already disabled!");
            return;
        } else {
            emit stopTaskHandControl();
        }
    }
}

void CommandCenter::startTaskHandControlCC(){
    if (this->isEnabled() == false){
        logger->info("Not enabled!");
        return;
    }

    if (this->taskhandcontrol->isEnabled() == true){
        logger->info("Taskhandcontrol already enabled!");
        return;
    }

    QTimer::singleShot(0, navi, SLOT(clearGoal()));
    logger ->info("Stop and deactivate all task - handcontrol active");
    emit stopAllTasks();
    emit resetTCL();
    wait(1000);

    if(this->activeTask != "taskHand"){
        if(this->activeTask != ""){
            // Reschedule aborted task
            QList<QString> temp;
            temp.append(activeTask);
            QString msg = "Reschedule "+activeTask;
            emit newMessage(msg);
            for(int i = 0; i < this->scheduleList.length(); i++){
                temp.append(this->scheduleList.at(i));
            }
            this->scheduleList.clear();
            for(int i = 0; i < temp.length(); i++){
                this->scheduleList.append(temp.at(i));
            }
        }

        // Now start taskhandcontrol
        emit startTaskHandControl();
        activeTask = "taskHand";
        logger->info("Set activeTask: taskHand");
    }
    emit setDepth(0.0);
    emit setForwardSpeed(0.0);
    emit setAngularSpeed(0.0);
    emit updateGUI();
}

void CommandCenter::handControlFinishedCC(RobotBehaviour *name, bool success){
    if (this->isEnabled() == false){
        logger->info("Not enabled!");
        return;
    }

    emit setAngularSpeed(0.0);
    emit setForwardSpeed(0.0);
    if(this->getSettingsValue("subEx").toBool() == true){
        emit setDepth(this->getSettingsValue("targetDepth").toFloat());
    } else {
        emit setDepth(0.0);
    }
    logger->info("Handcontrol has finished");
    if(success == true){

        this->finishedList.append(name->getTabName());
    } else {
        this->abortedList.append(name->getTabName());
    }
    // Stop thruster, then make next task...
    controlTimer.singleShot(this->getSettingsValue("waitTime").toInt(),this, SLOT(doNextTask()));

    activeTask = "";
    emit updateGUI();
}
