#include "commandcenter.h"
#include "commandcenterform.h"
#include <QtCore>
#include <stdio.h>
#include <Module_Simulation/module_simulation.h>
#include <CommandCenter/commandcenterform.h>
#include <Module_PressureSensor/module_pressuresensor.h>
#include <Module_ThrusterControlLoop/module_thrustercontrolloop.h>
#include <Module_HandControl/module_handcontrol.h>



CommandCenter::CommandCenter(QString id, Module_ThrusterControlLoop* tcl, Module_HandControl* handControl, Module_PressureSensor* pressure, Module_Simulation *sim, TestTask *tt, TestTask2 *tt2)
    : RobotModule(id)
{
    qDebug()<<"commandcenter thread id";
    qDebug()<< QThread::currentThreadId();

    this->tcl = tcl;
    this->handControl = handControl;
    this->pressure = pressure;
    this->sim = sim;
    this->testtask = tt;
    this->testtask2 = tt2;

    timer.moveToThread(this);

    setEnabled(false);
    running = false;

    connect(testtask, SIGNAL(finished(RobotBehaviour*,bool)), this, SLOT(finishedControl(RobotBehaviour*,bool)));
     connect(testtask2, SIGNAL(finished(RobotBehaviour*,bool)), this, SLOT(finishedControl(RobotBehaviour*,bool)));
    //connect(this, SIGNAL(startControl(RobotBehaviour*,bool)), this, SLOT(commandCenterControl(RobotBehaviour*,bool)));

     connect(this,SIGNAL(setDepth(float)),tcl,SLOT(setDepth(float)));
     connect(this,SIGNAL(setForwardSpeed(float)),tcl,SLOT(setForwardSpeed(float)));
     connect(this,SIGNAL(setAngularSpeed(float)),tcl,SLOT(setAngularSpeed(float)));
     connect(this,SIGNAL(resetTCL()),tcl,SLOT(reset()));

     connect(handControl, SIGNAL(emergencyStop()), this, SLOT(emergencyStop()));
     connect(handControl, SIGNAL(startHandControl()), this, SLOT(startHandControl()));

     connect(this,SIGNAL(startTestTask()),testtask,SLOT(startBehaviour()));
     connect(this,SIGNAL(stopTestTask()),testtask,SLOT(stop()));
     connect(this,SIGNAL(startTestTask2()),testtask2,SLOT(startBehaviour()));
     connect(this,SIGNAL(stopTestTask2()),testtask2,SLOT(stop()));


     connect(this,SIGNAL(stopAllTasks()),testtask,SLOT(emergencyStop()));
     connect(this,SIGNAL(stopAllTasks()),testtask2,SLOT(emergencyStop()));


     connect(this, SIGNAL(taskTimeout()), this, SLOT(timeout()));

     setDefaultValue("targetDepth",0.30);
     setDefaultValue("forwardSpeed",0.3);
     setDefaultValue("subEx", false);

     count = 1;

     depthWaitTimer.setSingleShot(true);
     depthWaitTimer.moveToThread(this);


     //controlTimer.setSingleShot(true);
     controlTimer.moveToThread(this);

     connect(this, SIGNAL(cStop()), this, SLOT(stopCC()));
     //connect(&depthWaitTimer, SIGNAL(timeout()), this, SLOT(stateTimeout()));

     //setDefaultValue("depthErrorVariance",0.05);
     //setDefaultValue("timeout",600);
}


bool CommandCenter::isActive()
{
    return isEnabled();
}

void CommandCenter::init(){

   logger->debug("cc init");

}

void CommandCenter::startCC(){
    RobotModule::reset();
    running = true;
    this->setEnabled(true);
    qDebug("CommandCenter started");
    logger->info("CommandCenter started");
    qDebug("Scheduled tasks:");
    for(int i = schedule.length()-1; i>=0; i--){
        qDebug()<<schedule.at(i);
    }

    if(this->getSettingsValue("subEx").toBool() == true){
        qDebug("Submerged execution");
        submergedExecute();
    }
    commandCenterControl();
}

void CommandCenter::stopCC(){
    RobotModule::reset();

    qDebug("CommandCenter stoped");
    logger->info("CommandCenter stoped");
    this->setHealthToSick("stopped command center!");
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

void CommandCenter::terminate(){
    RobotModule::terminate();
}

void CommandCenter::reset(){
    RobotModule::reset();

    if (sim->isEnabled())
    {

    }
}

QList<RobotModule*> CommandCenter::getDependencies(){
    QList<RobotModule*> ret;
    ret.append(sim);
    return ret;
}

QWidget* CommandCenter::createView(QWidget *parent)
{
    return new CommandCenterForm(this, parent);
}

void CommandCenter::commandCenterControl(){

    qDebug("Control cc");
   //depthWaitTimer.singleShot(6000,this, SLOT(timeout()));


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

    } else if(temp == "TestTask2") {
        emit startTestTask2();

        emit newList("");
        emit currentTask(temp);
        lTask = temp;
    } else {
        emit newError("Task not found, try to process next task...");
        emit newAborted(temp);
        commandCenterControl();
        qDebug("Schedule error, task not found!");
        cStop();
    }
    count++;
    } else {
        qDebug("Schedule error, no existing tasks!");

        emit newError("No existing task");
    }


}

void CommandCenter::finishedControl(RobotBehaviour *, bool success){

    if(success==true){
        qDebug("One task has finished successfully");
        emit newList(lTask);
    } else {
        qDebug("One task has finished unsuccessfully");
        emit newAborted(lTask);
        emit newList("");
    }
    if(this->isEnabled()){
        controlTimer.singleShot(5000,this, SLOT(doNextTask()) );

    }
}


void CommandCenter::doNextTask(){
    qDebug("Do next task");
    commandCenterControl();
}


void CommandCenter::timeout()
{
    qDebug("timeout in cc");
    stopCC();
    emit setDepth(0);
    emit setForwardSpeed(0);
    emit setAngularSpeed(0);
}

void CommandCenter::submergedExecute()
{
    emit setDepth(this->getSettingsValue("targetDepth").toFloat());
    qDebug()<<this->getSettingsValue("targetDepth").toString();
}
