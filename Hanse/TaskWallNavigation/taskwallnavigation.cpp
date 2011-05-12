#include "taskwallnavigation.h"
#include "taskwallnavigationform.h"
#include <QtGui>
#include <Module_Simulation/module_simulation.h>

TaskWallNavigation::TaskWallNavigation(QString id, Module_Simulation *sim, Behaviour_WallFollowing *w, Module_Navigation *n)
    : RobotBehaviour(id)
{
    qDebug()<<"taskwallnavigation thread id";
    qDebug()<< QThread::currentThreadId();
    this->sim = sim;
    this->wall = w;
    this->wall->setEnabled(false);
    this->navi = n;

    setEnabled(false);
    running = false;


    connect(this, SIGNAL(end()), this, SLOT(stop()));

    // Default settings
    this->setDefaultValue("forwardSpeed",0.5);
    this->setDefaultValue("angularSpeed",0.3);
    this->setDefaultValue("desiredDistance",1.5);
    this->setDefaultValue("corridorWidth",0.2);
    this->setDefaultValue("taskDuration",20000);
    this->setDefaultValue("description", "");

    this->setDefaultValue("startNavigation", "startN");
    this->setDefaultValue("startTolerance", 10);
    this->setDefaultValue("targetNavigation", "targetN");
    this->setDefaultValue("targetTolerance", 10);

    this->setDefaultValue("timerActivated", true);
    this->setDefaultValue("loopActivated", true);

    taskTimer.setSingleShot(true);
    taskTimer.moveToThread(this);

    distanceToStart = 0;
    distanceToTarget = 0;

}

bool TaskWallNavigation::isActive(){
    return isEnabled();
}


void TaskWallNavigation::init(){
    logger->debug("taskwallnavigation init");
}



void TaskWallNavigation::startBehaviour(){
    this->reset();
    logger->info("Taskwallnavigation started" );
    running = true;
    setHealthToOk();
    setEnabled(true);
    emit started(this);
    running = true;


    // Enable all components
    //    if(!this->wall->echo->isEnabled()){
    //        this->wall->echo->setEnabled(true);
    //    }

    //    if(!this->wall->isEnabled()){
    //        this->wall->setEnabled(true);
    //    }

    //    if(!this->navi->tcl->isEnabled()){
    //        this->navi->tcl->setEnabled(true);
    //    }

    //    if(!this->navi->sonarLoc->isEnabled()){
    //        this->navi->sonarLoc->setEnabled(true);
    //    }

    //    if(!this->navi->pressure->isEnabled()){
    //        this->navi->pressure->setEnabled(true);
    //    }

    //    if(!this->navi->mti->isEnabled()){
    //        this->navi->mti->setEnabled(true);
    //    }

    //    if(!this->navi->compass->isEnabled()){
    //        this->navi->compass->setEnabled(true);
    //    }

    if(!this->navi->isEnabled()){
        this->navi->setEnabled(true);
    }





    if(!this->navi->isEnabled()){
        this->navi->setEnabled(true);
    }



    //    addData("taskDuration", this->getSettingsValue("taskDuration"));
    //    addData("desiredDistance", this->wall->getSettingsValue("desiredDistance"));
    //    addData("forwardSpeed", this->wall->getSettingsValue("forwardSpeed"));
    //    addData("angularSpeed", this->wall->getSettingsValue("angularSpeed"));
    //    addData("corridorWidth", this->wall->getSettingsValue("corridorWidth"));
    //    emit dataChanged(this);
    addData("loop", this->getSettingsValue("loopActivated").toBool());
    addData("timer", this->getSettingsValue("timerActivated").toBool());
    addData("state", "start TaskWallNavigation");
    emit dataChanged(this);

    //    this->wall->echo->setEnabled(true);
    //    this->wall->startBehaviour();

    emit updateSettings();

    if(this->getSettingsValue("timerActivated").toBool()){
        qDebug("Taskwallnavigation with timer stop");
        taskTimer.singleShot(this->getSettingsValue("taskDuration").toInt(),this, SLOT(stop()));
        qDebug()<<this->getSettingsValue("taskDuration").toInt();
    }

    controlTask();

}

void TaskWallNavigation::controlTask(){
    distanceToStart = this->navi->getDistance(this->getSettingsValue("startNavigation").toString());

    addData("state", "move to start");
    emit dataChanged(this);
    // First navigate to start position
    while(distanceToStart > this->getSettingsValue("startTolerance").toDouble()){
        this->navi->gotoWayPoint(this->getSettingsValue("startNavigation").toString(), this->getSettingsValue("startTolerance").toDouble());
        qDebug("distance to start");
        qDebug()<<distanceToStart;
        msleep(500);
    }

    // Now do wallfollowing until target position is reached
    addData("state", "do WallFollowing");
    emit dataChanged(this);
    if(!this->wall->isEnabled()){
        qDebug("enable wallfollow and echo");
        this->wall->echo->setEnabled(true);
        this->wall->startBehaviour();
    }

    distanceToTarget = this->navi->getDistance(this->getSettingsValue("targetNavigation").toString());
    while(distanceToTarget > this->getSettingsValue("targetTolerance").toDouble()){
        msleep(500);
        qDebug("distance to target");
        qDebug()<<distanceToTarget;
    }

    addData("state", "WallFollowing finished");
    emit dataChanged(this);
    msleep(2500);

    if(this->wall->isEnabled()){
        qDebug("disable wallfollow and echo");
        this->wall->stop();
        this->wall->echo->setEnabled(false);
    }

    addData("state", " idle");
    emit dataChanged(this);
    msleep(2000);


    if(this->getSettingsValue("loopActivated").toBool()){
        controlTask();
    } else {
        msleep(10000);
        emit end();
    }

}



void TaskWallNavigation::stop(){
    running = false;
    logger->info("Taskwallnavigation stopped");

    if (this->isActive())
    {
        this->taskTimer.stop();
        //        this->wall->stop();
        //        this->wall->echo->setEnabled(false);
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
        this->taskTimer.stop();
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


