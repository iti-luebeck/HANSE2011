#include "commandcenter.h"
#include "commandcenterform.h"
#include <QtCore>
#include <stdio.h>
#include <Module_Simulation/module_simulation.h>
#include <CommandCenter/commandcenterform.h>

CommandCenter::CommandCenter(QString id, Module_Simulation *sim, TestTask *tt, TestTask2 *tt2)
    : RobotModule(id)
{
    qDebug()<<"commandcenter thread id";
    qDebug()<< QThread::currentThreadId();
    this->sim = sim;
    this->testtask = tt;
    this->testtask2 = tt2;
    timer.moveToThread(this);

    setEnabled(false);
    running = false;

    connect(testtask, SIGNAL(finished(RobotBehaviour*,bool)), this, SLOT(finishedControl(RobotBehaviour*,bool)));
     connect(testtask2, SIGNAL(finished(RobotBehaviour*,bool)), this, SLOT(finishedControl(RobotBehaviour*,bool)));
    //connect(this, SIGNAL(startControl(RobotBehaviour*,bool)), this, SLOT(commandCenterControl(RobotBehaviour*,bool)));
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
    this->enabled(true);
    qDebug("CommandCenter started");
    logger->info("CommandCenter started");
    qDebug("Scheduled tasks:");
    for(int i = schedule.length()-1; i>=0; i--){
        qDebug()<<schedule.at(i);
    }
    commandCenterControl();
}

void CommandCenter::stopCC(){
    RobotModule::reset();

    qDebug("CommandCenter stoped");
    logger->info("CommandCenter stoped");
    this->setHealthToSick("stopCC!");
    this->enabled(false);
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
    if(!schedule.isEmpty()){
    qDebug("Commandcenter control; Next scheduled task:");
    QString temp=schedule.last();
    qDebug()<<temp;

    if(temp == "TestTask"){
        testtask->startBehaviour();
        schedule.removeLast();
        emit nData(temp);
    } else if(temp == "TestTask2") {
        testtask2->startBehaviour();
        emit nData(temp);
        schedule.removeLast();
    }
} else {
        qDebug("Schedule error, no existing tasks!");
        emit nData("No existing task");
    }


}

void CommandCenter::finishedControl(RobotBehaviour *, bool success){
    qDebug("One task has finished");
    commandCenterControl();
}
