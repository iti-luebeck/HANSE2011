/*
   - Fahre zu Position A
   -  Mache dann Wandverfolgung bis Position B erreicht wird
       -- Falls Wandverfolgung nicht möglich, fahre direkt zu Position B
   - Wenn Position B erreicht, beende Task oder beginne von vorn
*/

#include "taskwallnavigation.h"
#include "taskwallnavigationform.h"
#include <QtGui>
#include <Module_Simulation/module_simulation.h>

TaskWallNavigation::TaskWallNavigation(QString id, Module_Simulation *sim, Behaviour_WallFollowing *w, Module_Navigation *n)
    : RobotBehaviour(id)
{
    this->sim = sim;
    this->wall = w;
    this->wall->setEnabled(false);
    this->navi = n;

    setEnabled(false);
    running = false;

    // Default settings
    this->setDefaultValue("forwardSpeed",0.5);
    this->setDefaultValue("angularSpeed",0.3);
    this->setDefaultValue("desiredDistance",1.5);
    this->setDefaultValue("corridorWidth",0.2);
    this->setDefaultValue("description", "");
    this->setDefaultValue("taskStopTime",120000);
    this->setDefaultValue("signalTimer",1000);


    this->setDefaultValue("startNavigation", "startN");
    this->setDefaultValue("targetNavigation", "targetN");
    this->setDefaultValue("targetTolerance", 10);

    this->setDefaultValue("timerActivated", true);
    this->setDefaultValue("loopActivated", true);

    taskTimer.setSingleShot(true);
    taskTimer.moveToThread(this);

    distanceToTarget = 0;

    connect(this, SIGNAL(enabled(bool)), this, SLOT(controlEnabledChanged(bool)));
    connect(navi, SIGNAL(reachedWaypoint(QString)), this, SLOT(seReached(QString)));

}

bool TaskWallNavigation::isActive(){
    return isEnabled();
}


void TaskWallNavigation::init(){
    logger->debug("taskwallnavigation init");
}



void TaskWallNavigation::startBehaviour(){
    if (this->isEnabled() == true){
        logger->info("Already enabled/started!");
        return;
    }
    this->reset();
    logger->info("Taskwallnavigation started" );
    running = true;
    setHealthToOk();
    setEnabled(true);
    emit started(this);
    running = true;

    emit newStateOverview("Move to start");
    emit newStateOverview("Do wallfollowing");
    emit newStateOverview("Move to end");
    if(this->getSettingsValue("loopActivated").toBool()){
        emit newStateOverview("...in a loop");
    }
    // Enable all components
    if(!this->navi->isEnabled()){
        this->navi->setEnabled(true);
    }

    addData("loop", this->getSettingsValue("loopActivated").toBool());
    addData("stoptimer", this->getSettingsValue("timerActivated").toBool());
    addData("state", "Start TaskWallNavigation");
    emit dataChanged(this);
    emit newState("Start TaskWallNavigation");

    emit updateSettings();

    if(this->getSettingsValue("timerActivated").toBool()){
        logger->debug("Taskwallnavigation with timer stop");
        taskTimer.singleShot(this->getSettingsValue("taskStopTime").toInt(),this, SLOT(timeoutStop()));
        taskTimer.start();
    }
    QTimer::singleShot(0, this, SLOT(moveToStart()));
}

void TaskWallNavigation::moveToStart(){
    if(this->isEnabled() && this->navi->getHealthStatus().isHealthOk()){
        logger->debug("move to start");
        addData("state", "Move to start");
        emit newState("Move to start");
        emit dataChanged(this);
        this->navi->gotoWayPoint(this->getSettingsValue("startNavigation").toString());
    } else {
        logger->info("Something is wrong with navigation/task, abort...");
        QTimer::singleShot(0, this, SLOT(emergencyStop()));
    }
}

void TaskWallNavigation::seReached(QString waypoint){
    if(this->isEnabled() == true){
        if(waypoint == this->getSettingsValue("startNavigation").toString()){
            // Start reached, do wallfollowing
            QTimer::singleShot(0, navi, SLOT(clearGoal()));
            QTimer::singleShot(0, this, SLOT(doWallFollow()));
        } else if(waypoint == this->getSettingsValue("targetNavigation").toString()){
            // End reached, control next state
            QTimer::singleShot(0, navi, SLOT(clearGoal()));
            QTimer::singleShot(0, this, SLOT(controlNextState()));
        } else {
            // Error!
            logger->info("Something is wrong with navigation waypoints, abort...");
            QTimer::singleShot(0, this, SLOT(emergencyStop()));
        }
    }
}

void TaskWallNavigation::doWallFollow(){
    if(this->isEnabled() && this->wall->getHealthStatus().isHealthOk()){
        logger->debug("Do wallfollowing");
        addData("state", "Do wallfollowing");
        emit newState("Do wallfollowing");
        emit dataChanged(this);
        this->wall->setSettingsValue("corridorWidth",this->getSettingsValue("corridorWidth").toFloat());
        this->wall->setSettingsValue("desiredDistance",this->getSettingsValue("desiredDistance").toFloat());
        if(!this->wall->isEnabled()){
            logger->debug("enable wallfollow and echo");
            QTimer::singleShot(0, wall, SLOT(startBehaviour()));
        }

        // Now do wallfollowing until target position is reached
        distanceToTarget = this->navi->getDistance(this->getSettingsValue("targetNavigation").toString());
        while(distanceToTarget > this->getSettingsValue("targetTolerance").toDouble()){
            addData("remaining distance",distanceToTarget);
            emit dataChanged(this);
            msleep(100);
        }

        addData("remaining distance", "-");
        emit dataChanged(this);
        addData("state", "Wallfollowing finished");
        emit dataChanged(this);

        // Target position reached, stop wallfollow
        if(this->wall->isEnabled()){
            logger->debug("Disable wallfollow and echo");
            QTimer::singleShot(0, wall, SLOT(stop()));
            this->wall->echo->setEnabled(false);
        }

        QTimer::singleShot(0, this, SLOT(controlNextState()));

    } else {
        logger->info("Something is wrong with wallfollow/task, moveToEnd...");
        QTimer::singleShot(0, this, SLOT(moveToEnd()));
    }
}

void TaskWallNavigation::moveToEnd(){
    if(this->isEnabled() && this->navi->getHealthStatus().isHealthOk()){
        logger->debug("Move to end");
        addData("state", "Move to end");
        emit dataChanged(this);
        emit newState("Move to end");

        this->navi->gotoWayPoint(this->getSettingsValue("startNavigation").toString());
    } else {
        logger->info("Something is wrong with navigation, abort...");
        QTimer::singleShot(0, this, SLOT(emergencyStop()));
    }
}

void TaskWallNavigation::controlNextState(){
    if(this->isEnabled()){
        logger->debug("idle, controlNextState");
        addData("state", "Idle, control next state");
        emit dataChanged(this);
        emit newState("Idle, control next state");

        if(this->getSettingsValue("loopActivated").toBool()){
            logger->debug("start again");
            QTimer::singleShot(0, this, SLOT(moveToStart()));
        } else {
            // Task finished, stop
            QTimer::singleShot(0, this, SLOT(stop()));
        }
    } else {
        logger->info("Something is wrong with the task, abort...");
        QTimer::singleShot(0, this, SLOT(emergencyStop()));
    }
}


void TaskWallNavigation::stop(){
    if(this->isEnabled()){
        running = false;
        logger->info("Taskwallnavigation stopped");

        if (this->isActive())
        {
            this->taskTimer.stop();

            this->navi->setEnabled(false);
            this->wall->echo->setEnabled(false);
            QTimer::singleShot(0, wall, SLOT(stop()));

            this->setEnabled(false);
            emit finished(this,true);
        }
    } else {
        logger->info("Something is really wrong...");
        emit finished(this,false);
    }
}

void TaskWallNavigation::timeoutStop(){
    if(this->isEnabled()){
        running = false;
        logger->info("Taskwallnavigation timeout stopped");

        if (this->isActive())
        {
            this->taskTimer.stop();
            this->navi->setEnabled(false);
            this->wall->echo->setEnabled(false);
            QTimer::singleShot(0, wall, SLOT(stop()));

            this->setEnabled(false);
            emit finished(this,false);
        }
    }
}

void TaskWallNavigation::emergencyStop(){
    if (this->isEnabled() == false){
        logger->info("Not enabled!");
        return;
    }
    running = false;
    logger->info( "Taskwallnavigation emergency stopped" );

    if (this->isActive())
    {
        this->taskTimer.stop();
        this->navi->setEnabled(false);
        this->wall->echo->setEnabled(false);
        QTimer::singleShot(0, wall, SLOT(stop()));
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

void TaskWallNavigation::controlEnabledChanged(bool b){
    if(b == false){
        logger->info("No longer enabled!");
        QTimer::singleShot(0, this, SLOT(emergencyStop()));
    }
}
