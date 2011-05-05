#include "taskwallfollowing.h"
#include "taskwallfollowingform.h"
#include <QtGui>
#include <Module_Simulation/module_simulation.h>

TaskWallFollowing::TaskWallFollowing(QString id, Behaviour_WallFollowing *w, Module_Simulation *sim)
    : RobotBehaviour(id)
{
    qDebug()<<"testwallfollowing thread id";
    qDebug()<< QThread::currentThreadId();
    this->sim = sim;
    this->wall = w;
    this->wall->setEnabled(false);

    setEnabled(false);
    running = false;

    // Default settings
    this->setDefaultValue("forwardSpeed1",0.5);
    this->setDefaultValue("angularSpeed1",0.3);
    this->setDefaultValue("desiredDistance1",1.5);
    this->setDefaultValue("corridorWidth1",0.2);
    this->setDefaultValue("taskDuration1",30000);
    this->setDefaultValue("description1", "task 1");

    this->setDefaultValue("forwardSpeed2",0.4);
    this->setDefaultValue("angularSpeed2",0.1);
    this->setDefaultValue("desiredDistance2",2.0);
    this->setDefaultValue("corridorWidth2",0.2);
    this->setDefaultValue("taskDuration2",20000);
    this->setDefaultValue("description2", "task 2");

    this->setDefaultValue("forwardSpeed3",0.2);
    this->setDefaultValue("angularSpeed3",0.1);
    this->setDefaultValue("desiredDistance3",1.0);
    this->setDefaultValue("corridorWidth3",0.0);
    this->setDefaultValue("taskDuration3",10000);
    this->setDefaultValue("description3", "task 3");

    testTimer.setSingleShot(true);
    testTimer.moveToThread(this);
}

bool TaskWallFollowing::isActive(){
    return isEnabled();
}


void TaskWallFollowing::init(){
    logger->debug("testwallfollowing init");
}



void TaskWallFollowing::startBehaviour(){
    this->reset();
    logger->info("Taskwallfollowing started" );
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

void TaskWallFollowing::setRunData(int taskNr){
    if(taskNr == 3){
        this->setSettingsValue("taskDuration", this->getSettingsValue("taskDuration3"));
        this->wall->setSettingsValue("desiredDistance", this->getSettingsValue("desiredDistance3"));
        this->wall->setSettingsValue("forwardSpeed", this->getSettingsValue("forwardSpeed3"));
        this->wall->setSettingsValue("angularSpeed", this->getSettingsValue("angularSpeed3"));
        this->wall->setSettingsValue("corridorWidth", this->getSettingsValue("corridorWidth3"));
    } else if (taskNr == 2){
        this->setSettingsValue("taskDuration", this->getSettingsValue("taskDuration2"));
        this->wall->setSettingsValue("desiredDistance", this->getSettingsValue("desiredDistance2"));
        this->wall->setSettingsValue("forwardSpeed", this->getSettingsValue("forwardSpeed2"));
        this->wall->setSettingsValue("angularSpeed", this->getSettingsValue("angularSpeed2"));
        this->wall->setSettingsValue("corridorWidth", this->getSettingsValue("corridorWidth2"));
    } else {
        this->setSettingsValue("taskDuration", this->getSettingsValue("taskDuration1"));
        this->wall->setSettingsValue("desiredDistance", this->getSettingsValue("desiredDistance1"));
        this->wall->setSettingsValue("forwardSpeed", this->getSettingsValue("forwardSpeed1"));
        this->wall->setSettingsValue("angularSpeed", this->getSettingsValue("angularSpeed1"));
        this->wall->setSettingsValue("corridorWidth", this->getSettingsValue("corridorWidth1"));
    }
}

void TaskWallFollowing::stop(){
    running = false;
    logger->info( "Task wallfollowing stopped" );

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
    logger->info( "Task wallfollowing emergency stopped" );

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
    ret.append(wall);
    ret.append(sim);
    return ret;
}


void TaskWallFollowing::newSchDesSlot(QString taskName,  QString newD){
    emit newSchDesSignal(taskName, newD);
}

void TaskWallFollowing::setDescriptionSlot(){
    emit setDescriptionSignal();
}
