#include "commandcenter.h"
#include "commandcenterform.h"
#include <QtCore>
#include <stdio.h>
#include <Module_Simulation/module_simulation.h>

CommandCenter::CommandCenter(QString id, Module_Simulation *sim)
    : RobotModule(id)
{
    this->sim = sim;

    timer.moveToThread(this);
}

CommandCenter::~CommandCenter(){
}

void CommandCenter::init(){

    connect(this, SIGNAL(enabled(bool)), this, SLOT(gotEnabledChanged(bool)));
}


void CommandCenter::terminate(){
    RobotModule::terminate();
}


void CommandCenter::run(void){
    running = true;

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


void CommandCenter::gotEnabledChanged(bool state)
{
    if(!state){
        reset();
    }
}
