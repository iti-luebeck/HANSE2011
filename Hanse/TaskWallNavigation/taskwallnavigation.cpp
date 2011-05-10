#include "taskwallnavigation.h"
#include "taskwallnavigationform.h"
#include <QtGui>
#include <Module_Simulation/module_simulation.h>

TaskWallNavigation::TaskWallNavigation(QString id, Behaviour_WallFollowing *w, Module_Simulation *sim)
    : RobotBehaviour(id)
{
    qDebug()<<"testwallnavigation thread id";
    qDebug()<< QThread::currentThreadId();
    this->sim = sim;
    this->wall = w;
    this->wall->setEnabled(false);

    setEnabled(false);
    running = false;

    // Default settings
    this->setDefaultValue("forwardSpeed",0.5);
    this->setDefaultValue("angularSpeed",0.3);
    this->setDefaultValue("desiredDistance",1.5);
    this->setDefaultValue("corridorWidth",0.2);
    this->setDefaultValue("taskDuration",30000);
    this->setDefaultValue("description", "task 1");

    this->setDefaultValue("startNavigation", "start");
    this->setDefaultValue("targetNavigation", "target");

    testTimer.setSingleShot(true);
    testTimer.moveToThread(this);

    connect(this, SIGNAL(setRunDataSignal(int)), this, SLOT(setRunData(int)));
}

bool TaskWallNavigation::isActive(){
    return isEnabled();
}


void TaskWallNavigation::init(){
    logger->debug("testwallfollowing init");
}



void TaskWallNavigation::startBehaviour(){
    this->reset();
    logger->info("Taskwallnavigation started" );
    running = true;
    setHealthToOk();
    setEnabled(true);
    emit started(this);
    running = true;
    addData("taskDuration", this->getSettingsValue("taskDuration"));
    addData("desiredDistance", this->wall->getSettingsValue("desiredDistance"));
    addData("forwardSpeed", this->wall->getSettingsValue("forwardSpeed"));
    addData("angularSpeed", this->wall->getSettingsValue("angularSpeed"));
    addData("corridorWidth", this->wall->getSettingsValue("corridorWidth"));
    emit dataChanged(this);
    this->wall->echo->setEnabled(true);
    this->wall->startBehaviour();
    testTimer.singleShot(this->getSettingsValue("taskDuration").toInt(),this, SLOT(stop()));
}


void TaskWallNavigation::stop(){
    running = false;
    logger->info( "Taskwallnavigation stopped" );

    if (this->isActive())
    {
        this->testTimer.stop();
        this->wall->stop();
        this->wall->echo->setEnabled(false);
        this->setEnabled(false);
        emit finished(this,true);
    }
}


void TaskWallNavigation::emergencyStop()
{
    running = false;
    logger->info( "Taskwallnavigation emergency stopped" );

    if (this->isActive())
    {
        this->testTimer.stop();
        this->wall->stop();
        this->wall->echo->setEnabled(false);
        this->setEnabled(false);
        emit finished(this,false);
    }
}


void TaskWallNavigation::terminate()
{
    this->stop();
    RobotModule::terminate();
}

QWidget* TaskWallNavigation::createView(QWidget *parent)
{
    return new TaskWallNavigationForm(this, parent);
}

QList<RobotModule*> TaskWallNavigation::getDependencies()
{
    QList<RobotModule*> ret;
    ret.append(wall);
    ret.append(sim);
    return ret;
}


