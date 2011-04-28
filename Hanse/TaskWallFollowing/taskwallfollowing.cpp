#include "taskwallfollowing.h"
#include "taskwallfollowingform.h"
#include <QtGui>
#include <Module_Simulation/module_simulation.h>

TaskWallFollowing::TaskWallFollowing(QString id, Behaviour_WallFollowing *w, Module_Simulation *sim)
    : RobotBehaviour(id)
{
    qDebug()<<"testtask thread id";
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

    testTimer.setSingleShot(true);
    testTimer.moveToThread(this);
}

bool TaskWallFollowing::isActive(){
    return isEnabled();
}


void TaskWallFollowing::init(){
    logger->debug("testtask init");
}



void TaskWallFollowing::startBehaviour(){
    // Get new settings
    setWallSettings();

    this->reset();
    logger->info("TestTastk started" );
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

void TaskWallFollowing::setWallSettings(){
    this->wall->setSettingsValue("desiredDistance", this->getSettingsValue("desiredDistance"));
    this->wall->setSettingsValue("forwardSpeed", this->getSettingsValue("forwardSpeed"));
    this->wall->setSettingsValue("angularSpeed", this->getSettingsValue("angularSpeed"));
    this->wall->setSettingsValue("corridorWidth", this->getSettingsValue("corridorWidth"));
}

void TaskWallFollowing::stop(){
    running = false;
    logger->info( "testTask stopped" );

    if (this->isActive())
    {
        this->testTimer.stop();
        this->wall->stop();
        this->wall->echo->setEnabled(false);
        this->setEnabled(false);
        emit finished(this,true);
    }
}


void TaskWallFollowing::emergencyStop()
{
    running = false;
    logger->info( "testTask emergency stopped" );

    if (this->isActive())
    {
        this->testTimer.stop();
        this->wall->stop();
        this->wall->echo->setEnabled(false);
        this->setEnabled(false);
        emit finished(this,false);
    }
}


void TaskWallFollowing::terminate()
{
    this->stop();
    RobotModule::terminate();
}

QWidget* TaskWallFollowing::createView(QWidget *parent)
{
    return new TaskWallFollowingForm(this, parent);
}

QList<RobotModule*> TaskWallFollowing::getDependencies()
{
    QList<RobotModule*> ret;
    //ret.append(tcl);
    ret.append(sim);
    return ret;
}


