#include "commandcenter.h"
#include "commandcenterform.h"
#include <QtCore>
#include <stdio.h>
#include <Module_Simulation/module_simulation.h>
#include <CommandCenter/commandcenterform.h>
#include <Module_PressureSensor/module_pressuresensor.h>
#include <Module_ThrusterControlLoop/module_thrustercontrolloop.h>
#include <Module_HandControl/module_handcontrol.h>

CommandCenter::CommandCenter(QString id, Module_ThrusterControlLoop* tcl, Module_HandControl* handControl, Module_PressureSensor* pressure, Module_Simulation *sim, TestTask *tt, TaskWallFollowing *twf, TaskThrusterControl *ttc, TaskPipeFollowing *tpf)
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
    this->taskthrustercontrol = ttc;
    this->taskpipefollowing = tpf;


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

    // Taskwallfollowing
    connect(taskwallfollowing, SIGNAL(finished(RobotBehaviour*,bool)), this, SLOT(finishedControl(RobotBehaviour*,bool)));
    connect(this,SIGNAL(startTaskWallFollowing()),taskwallfollowing,SLOT(startBehaviour()));
    connect(this,SIGNAL(stopTaskWallFollowing()),taskwallfollowing,SLOT(stop()));
    connect(this,SIGNAL(stopAllTasks()),taskwallfollowing,SLOT(emergencyStop()));
    connect(this,SIGNAL(setTaskWallFollowing(int)),taskwallfollowing,SLOT(setRunData(int)));
    connect(taskwallfollowing, SIGNAL(newSchDesSignal(QString, QString)), this, SLOT(newSchDesSlot(QString, QString)));
    connect(this,SIGNAL(setDescriptionSignal()),taskwallfollowing,SLOT(setDescriptionSlot()));

    // Taskthrustercontrol
    connect(taskthrustercontrol, SIGNAL(finished(RobotBehaviour*,bool)), this, SLOT(finishedControl(RobotBehaviour*,bool)));
    connect(this,SIGNAL(startTaskThrusterControl()),taskthrustercontrol,SLOT(startBehaviour()));
    connect(this,SIGNAL(stopTaskThrusterControl()),taskthrustercontrol,SLOT(stop()));
    connect(this,SIGNAL(stopAllTasks()),taskthrustercontrol,SLOT(emergencyStop()));
    connect(this,SIGNAL(setTaskThrusterControl(int)),taskthrustercontrol,SLOT(setRunData(int)));
    connect(taskthrustercontrol, SIGNAL(newMessage(QString)), this, SLOT(setNewMessage(QString)));
    connect(taskthrustercontrol, SIGNAL(newSchDesSignal(QString, QString)), this, SLOT(newSchDesSlot(QString, QString)));
    connect(this,SIGNAL(setDescriptionSignal()),taskthrustercontrol,SLOT(setDescriptionSlot()));


    // Taskpipefollowing
    connect(taskpipefollowing, SIGNAL(finished(RobotBehaviour*,bool)), this, SLOT(finishedControl(RobotBehaviour*,bool)));
    connect(this,SIGNAL(startTaskPipeFollowing()),taskpipefollowing,SLOT(startBehaviour()));
    connect(this,SIGNAL(stopTaskPipeFollowing()),taskpipefollowing,SLOT(stop()));
    connect(this,SIGNAL(stopAllTasks()),taskpipefollowing,SLOT(emergencyStop()));
    connect(this,SIGNAL(setTaskPipeFollowing(int)),taskpipefollowing,SLOT(setRunData(int)));
    connect(taskpipefollowing, SIGNAL(newSchDesSignal(QString, QString)), this, SLOT(newSchDesSlot(QString, QString)));
    connect(this,SIGNAL(setDescriptionSignal()),taskpipefollowing,SLOT(setDescriptionSlot()));

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
}

void CommandCenter::startCommandCenter(){
    RobotModule::reset();
    running = true;
    this->setEnabled(true);

    // Every task needs thruster
    this->tcl->setEnabled(true);

    logger->info("CommandCenter started");

    if(this->getSettingsValue("subEx").toBool() == true){
        // qDebug("Submerged execution!");
        addData("Submerged",true);
        emit dataChanged(this);
        submergedExecute();
    } else {
        addData("Submerged",false);
        emit dataChanged(this);
        this->taskthrustercontrol->setSettingsValue("commandCenterSubExecuteDepth", 0.0);
        qDebug("Set taskthrustercontrol finishing depth");
        qDebug()<<this->taskthrustercontrol->getSettingsValue("commandCenterSubExecuteDepth").toString();
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

    emit setDepth(0.0);
    emit setForwardSpeed(0.0);
    emit setAngularSpeed(0.0);

    if (sim->isEnabled())
    {

    }
}

void CommandCenter::commandCenterControl(){
    if(!schedule.isEmpty() & this->isEnabled()){
        qDebug("Commandcenter control; Next scheduled task:");
        QString temp=schedule.last();
        QString tempAkt = "";
        //qDebug()<<temp;
        for(int j = 0; j < temp.length(); j++){
            if(((temp[j] == QChar(':'))==false) && ((temp[j] == QChar(' '))==false) ){
                tempAkt = tempAkt + temp.at(j);
            } else {
                break;
            }
        }
        qDebug()<<tempAkt;
        schedule.removeLast();
        addData("Task Nr", count);
        addData("Task Name", tempAkt);
        emit dataChanged(this);
        if(tempAkt == "TestTask"){
            emit startTestTask();
            emit newList("");
            emit currentTask(tempAkt);
            lTask = tempAkt;
        } else if(tempAkt == "TaskWallFollowing1"){
            emit setTaskWallFollowing(1);
            emit startTaskWallFollowing();
            emit newList("");
            emit currentTask(tempAkt);
            lTask = tempAkt;
        } else if(tempAkt == "TaskWallFollowing2"){
            emit setTaskWallFollowing(2);
            emit startTaskWallFollowing();
            emit newList("");
            emit currentTask(tempAkt);
            lTask = tempAkt;
        } else if(tempAkt == "TaskWallFollowing3"){
            emit setTaskWallFollowing(3);
            emit startTaskWallFollowing();
            emit newList("");
            emit currentTask(tempAkt);
            lTask = tempAkt;
        } else if(tempAkt == "TaskThrusterControl1"){
            emit setTaskThrusterControl(1);
            emit startTaskThrusterControl();
            emit newList("");
            emit currentTask(tempAkt);
            lTask = tempAkt;
        } else if(tempAkt == "TaskThrusterControl2"){
            emit setTaskThrusterControl(2);
            emit startTaskThrusterControl();
            emit newList("");
            emit currentTask(tempAkt);
            lTask = tempAkt;
        } else if(tempAkt == "TaskThrusterControl3"){
            emit setTaskThrusterControl(3);
            emit startTaskThrusterControl();
            emit newList("");
            emit currentTask(tempAkt);
            lTask = tempAkt;
        } else if(tempAkt == "TaskThrusterControl4"){
            emit setTaskThrusterControl(4);
            emit startTaskThrusterControl();
            emit newList("");
            emit currentTask(tempAkt);
            lTask = tempAkt;
        } else if(tempAkt == "TaskThrusterControl5"){
            emit setTaskThrusterControl(5);
            emit startTaskThrusterControl();
            emit newList("");
            emit currentTask(tempAkt);
            lTask = tempAkt;
        } else if(tempAkt == "TaskThrusterControl6"){
            emit setTaskThrusterControl(6);
            emit startTaskThrusterControl();
            emit newList("");
            emit currentTask(tempAkt);
            lTask = tempAkt;  
        } else if(tempAkt == "TaskPipeFollowing1"){
            emit setTaskPipeFollowing(1);
            emit startTaskPipeFollowing();
            emit newList("");
            emit currentTask(tempAkt);
            lTask = tempAkt;
        } else if(tempAkt == "TaskPipeFollowing2"){
            emit setTaskPipeFollowing(2);
            emit startTaskPipeFollowing();
            emit newList("");
            emit currentTask(tempAkt);
            lTask = tempAkt;
        } else if(tempAkt == "TaskPipeFollowing3"){
            emit setTaskPipeFollowing(3);
            emit startTaskPipeFollowing();
            emit newList("");
            emit currentTask(tempAkt);
            lTask = tempAkt;
        } else  {
            qDebug("Task not found, skip task!");
            emit newError("Task not found, skip task!");
            emit newAborted(temp);
            commandCenterControl();
        }
        count++;
    } else {
        qDebug("Schedule error, no existing tasks!");
        if(!this->getSettingsValue("subEx").toBool()){
            emit newError("No existing task");
        } else {
            emit newError("No existing task, surface");
        }
        cStop();
    }
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
    this->taskthrustercontrol->setSettingsValue("commandCenterSubExecuteDepth", this->getSettingsValue("targetDepth"));
    qDebug("Set taskthrustercontrol finishing depth");
    qDebug()<<this->taskthrustercontrol->getSettingsValue("commandCenterSubExecuteDepth").toString();
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

void CommandCenter::setNewMessage(QString s){
    emit newMessage(s);
    qDebug()<<s;
}


QList<RobotModule*> CommandCenter::getDependencies(){
    QList<RobotModule*> ret;
    ret.append(sim);
    return ret;
}

QWidget* CommandCenter::createView(QWidget *parent){
    return new CommandCenterForm(this, parent);
}


void CommandCenter::newSchDesSlot(QString scheduleName, QString newSD){
    emit newSchDesSignal(scheduleName, newSD);
}

void CommandCenter::setDescriptionSlot(){
    emit setDescriptionSignal();
}
