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
    this->setDefaultValue("startTolerance", 10);
    this->setDefaultValue("targetNavigation", "targetN");
    this->setDefaultValue("targetTolerance", 10);

    this->setDefaultValue("timerActivated", true);
    this->setDefaultValue("loopActivated", true);

    taskTimer.setSingleShot(true);
    taskTimer.moveToThread(this);

    distanceToStart = 0;
    distanceToTarget = 0;

    taskTimer.moveToThread(this);
    moveToStartTimer.moveToThread(this);
    moveToEndTimer.moveToThread(this);
    doWallFollowTimer.moveToThread(this);
    controlNextStateTimer.moveToThread(this);
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


    // Enable all components
    if(!this->navi->isEnabled()){
        this->navi->setEnabled(true);
    }

    addData("loop", this->getSettingsValue("loopActivated").toBool());
    addData("timer", this->getSettingsValue("timerActivated").toBool());
    addData("state", "start TaskWallNavigation");
    emit dataChanged(this);

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
        emit dataChanged(this);

        // First navigate to start position
        distanceToStart = this->navi->getDistance(this->getSettingsValue("startNavigation").toString());
        if(distanceToStart > this->getSettingsValue("startTolerance").toDouble()){
            addData("remaining distance", distanceToStart);
            emit dataChanged(this);
            moveToStartTimer.singleShot(this->getSettingsValue("signalTimer").toInt(),this, SLOT(moveToStart()));
        } else {
            addData("remaining distance", "-");
            emit dataChanged(this);
            addData("state", "Startposition reached");
            emit dataChanged(this);
            QTimer::singleShot(0, this, SLOT(doWallFollow()));
        }
    } else {
        logger->info("Something is wrong with navigation, abort...");
        QTimer::singleShot(0, this, SLOT(emergencyStop()));
    }
}
void TaskWallNavigation::doWallFollow(){
    if(this->isEnabled() && this->wall->getHealthStatus().isHealthOk()){
        logger->debug("Do wallfollowing");
        addData("state", "Do wallfollowing");
        emit dataChanged(this);
        this->wall->setSettingsValue("corridorWidth",this->getSettingsValue("corridorWidth").toFloat());
        this->wall->setSettingsValue("desiredDistance",this->getSettingsValue("desiredDistance").toFloat());
        if(!this->wall->isEnabled()){
            logger->debug("enable wallfollow and echo");
            QTimer::singleShot(0, wall, SLOT(startBehaviour()));
        }

        // Now do wallfollowing until target position is reached
        distanceToTarget = this->navi->getDistance(this->getSettingsValue("targetNavigation").toString());
        if(distanceToTarget > this->getSettingsValue("targetTolerance").toDouble()){
            addData("remaining distance",distanceToTarget);
            emit dataChanged(this);

            // Too many errors, abort wallfollowing and move to endposition
            if(this->wall->badDataCount > 100){
                QTimer::singleShot(0, this, SLOT(moveToEnd()));
            } else {
                doWallFollowTimer.singleShot(this->getSettingsValue("signalTimer").toInt(),this, SLOT(doWallFollow()));
            }
        } else {
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
        }
    } else {
        logger->info("Something is wrong with wallfollow, abort...");
        QTimer::singleShot(0, this, SLOT(emergencyStop()));
    }
}

void TaskWallNavigation::moveToEnd(){
    if(this->isEnabled() && this->navi->getHealthStatus().isHealthOk()){
        logger->debug("Move to end");
        addData("state", "Move to end");
        emit dataChanged(this);

        distanceToTarget = this->navi->getDistance(this->getSettingsValue("targetNavigation").toString());
        if(distanceToTarget > this->getSettingsValue("targetTolerance").toDouble()){
            addData("remaining distance", distanceToTarget);
            emit dataChanged(this);
            moveToEndTimer.singleShot(this->getSettingsValue("signalTimer").toInt(),this, SLOT(moveToEnd()));
        } else {
            QTimer::singleShot(0, this, SLOT(controlNextState()));
        }
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

        if(this->getSettingsValue("loopActivated").toBool()){
            logger->debug("start again");
            QTimer::singleShot(0, this, SLOT( moveToStart()));
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


