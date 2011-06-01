/*
    - Fahre zu Position Start
        - Falls nicht moeglich, emergencyStop
    - Mache x milisekunden Xsens-Following, dann einen Turn
        - Falls nicht moeglich, state ueberspringen
    - Fahre zu Position B
        - Falls nicht moeglich, emergencyStop
    - Mache Turn 180
        - Falls nicht moeglich, state ueberspringen
    - Fahre zu Position End
        - Falls nicht moeglich, emergencyStop
*/

#include "taskxsensnavigation.h"
#include "taskxsensnavigationform.h"
#include <QtGui>
#include <Module_Simulation/module_simulation.h>

TaskXsensNavigation::TaskXsensNavigation(QString id, Module_Simulation *sim, Behaviour_XsensFollowing *xf, Module_Navigation *n, Behaviour_TurnOneEighty *o180)
    : RobotBehaviour(id)
{
    this->sim = sim;
    this->xsensfollow = xf;
    this->navi = n;
    this->turn180 = o180;

    setEnabled(false);
    running = false;

    // Default task settings
    this->setDefaultValue("description", "");
    this->setDefaultValue("taskStopTime",120000);
    this->setDefaultValue("signalTimer",1000);

    this->setDefaultValue("startNavigation", "startN");
    this->setDefaultValue("startTolerance", 10);
    this->setDefaultValue("targetNavigation", "targetN");
    this->setDefaultValue("targetTolerance", 10);

    this->setDefaultValue("bNavigation", "bN");
    this->setDefaultValue("bTolerance", 10);

    this->setDefaultValue("timerActivated", true);
    this->setDefaultValue("loopActivated", true);

    // Default xsensfollow settings
    this->setDefaultValue("ffSpeed", 10);
    this->setDefaultValue("kp", 10);
    this->setDefaultValue("delta", 10);
    this->setDefaultValue("timer", 30);
    this->setDefaultValue("driveTime", 10000);

    // Default turn180 settings
    this->setDefaultValue("hysteresis", 10);
    this->setDefaultValue("p", 0.4);

    taskTimer.setSingleShot(true);
    taskTimer.moveToThread(this);

    distanceToStart = 0;
    distanceToTarget = 0;

    taskTimer.moveToThread(this);
    moveToStartTimer.moveToThread(this);
    moveToEndTimer.moveToThread(this);
    doXsensFollowTimer.moveToThread(this);
    controlNextStateTimer.moveToThread(this);

    connect(this, SIGNAL(enabled(bool)), this, SLOT(controlEnabledChanged(bool)));
}

bool TaskXsensNavigation::isActive(){
    return isEnabled();
}


void TaskXsensNavigation::init(){
    logger->debug("taskxsensnavigation init");
}



void TaskXsensNavigation::startBehaviour(){
    if (this->isEnabled() == true){
        logger->info("Already enabled/started!");
        return;
    }
    this->reset();
    logger->info("Taskxsensnavigation started" );
    running = true;
    setHealthToOk();
    setEnabled(true);
    emit started(this);
    running = true;


    // Stateoverview
    emit newStateOverview("Move to start");
    emit newStateOverview("Do xsensfollowing");
    emit newStateOverview("Move to B");
    emit newStateOverview("Turn180");
    emit newStateOverview("Move to end");
    if(this->getSettingsValue("loopActivated").toBool()){
            emit newStateOverview("...in a loop");
    }
    // Enable all components
    if(!this->navi->isEnabled()){
        this->navi->setEnabled(true);
    }

    addData("loop", this->getSettingsValue("loopActivated").toBool());
    addData("timer", this->getSettingsValue("timerActivated").toBool());
    addData("state", "Start TaskXsensNavigation");
    emit newState("Start TaskXsensNavigation");
    emit dataChanged(this);

    emit updateSettings();

    if(this->getSettingsValue("timerActivated").toBool()){
        logger->debug("Taskxsensnavigation with timer stop");
        taskTimer.singleShot(this->getSettingsValue("taskStopTime").toInt(),this, SLOT(timeoutStop()));
        taskTimer.start();
    }

    QTimer::singleShot(0, this, SLOT(moveToStart()));
}

void TaskXsensNavigation::moveToStart(){
    if(this->isEnabled() && this->navi->getHealthStatus().isHealthOk()){
        logger->debug("move to start");
        addData("state", "Move to start");
        emit dataChanged(this);
        emit newState("Move to start");
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
            QTimer::singleShot(0, this, SLOT(doXsensFollow()));
        }
    } else {
        logger->info("moveToStart not possible!");
        QTimer::singleShot(0, this, SLOT(emergencyStop()));
    }
}
void TaskXsensNavigation::doXsensFollow(){
    if(this->isEnabled() && this->xsensfollow->getHealthStatus().isHealthOk()){
        logger->debug("Do xsensfollowing");
        addData("state", "Do xsensfollowing");
        emit dataChanged(this);
        emit newState("Do xsensfollowing");

        if(!this->xsensfollow->isEnabled()){
            logger->debug("enable xsensfollow");
            this->xsensfollow->setSettingsValue("ffSpeed", this->getSettingsValue("ffSpeed").toString());
            this->xsensfollow->setSettingsValue("kp", this->getSettingsValue("kp").toString());
            this->xsensfollow->setSettingsValue("delta", this->getSettingsValue("delta").toString());
            this->xsensfollow->setSettingsValue("timer", this->getSettingsValue("timer").toString());

            this->xsensfollow->setSettingsValue("turnClockwise", true);
            this->xsensfollow->setSettingsValue("driveTime", this->getSettingsValue("driveTime").toString());
            QTimer::singleShot(0, xsensfollow, SLOT(startBehaviour()));
        }

        addData("Time until turn90",this->getSettingsValue("driveTime").toInt());
        emit dataChanged(this);

        // Finish state after drivetime msec and a little bit time to finish turn
        int tempWait = this->getSettingsValue("driveTime").toInt()+10000;
        QTimer::singleShot(tempWait, this, SLOT(finishXsensFollow()));
    } else {
        logger->info("doXsensFollow not possible!");
        QTimer::singleShot(0, this, SLOT(moveToB()));
    }
}

void TaskXsensNavigation::finishXsensFollow(){
    if(this->isEnabled() && this->xsensfollow->getHealthStatus().isHealthOk()){
        this->xsensfollow->setSettingsValue("turnClockwise", false);
        QTimer::singleShot(0, xsensfollow, SLOT(stop()));
        QTimer::singleShot(0, this, SLOT(moveToB()));
    } else {
        logger->info("finishXsensFollow not possible!");
        QTimer::singleShot(0, this, SLOT(moveToB()));       
    }
}

void TaskXsensNavigation::moveToB(){
    if(this->isEnabled() && this->navi->getHealthStatus().isHealthOk()){
        logger->debug("move to b");
        addData("state", "Move to B");
        emit dataChanged(this);
        emit newState("Move to B");
        // Navigate to B position
        distanceToB = this->navi->getDistance(this->getSettingsValue("bNavigation").toString());
        if(distanceToB > this->getSettingsValue("bTolerance").toDouble()){
            addData("remaining distance", distanceToB);
            emit dataChanged(this);
            moveToBTimer.singleShot(this->getSettingsValue("signalTimer").toInt(),this, SLOT(moveToB()));
        } else {
            addData("remaining distance", "-");
            emit dataChanged(this);
            addData("state", "B-Position reached");
            emit dataChanged(this);

            QTimer::singleShot(0, this, SLOT(doTurn()));
        }
    } else {
        logger->info("moveToB not possible!");
        QTimer::singleShot(0, this, SLOT(emergencyStop()));
    }
}


void TaskXsensNavigation::doTurn(){
    if(this->isEnabled()){
        logger->debug("turn180");
        addData("state", "Turn180");
        emit dataChanged(this);
        emit newState("Turn180");

        if(!this->turn180->isEnabled()){
            logger->debug("enable turn180");
            this->turn180->setSettingsValue("hysteresis", this->getSettingsValue("hysteresis").toInt());
            this->turn180->setSettingsValue("p", this->getSettingsValue("p").toInt());
            QTimer::singleShot(0, turn180, SLOT(startBehaviour()));
        }

        // Do Turn180. If its finished, it will be disabled
        if(this->turn180->isEnabled()){
            doTurnTimer.singleShot(this->getSettingsValue("signalTimer").toInt(),this, SLOT(doTurn()));
        } else {
            addData("state", "Turn180 finished");
            emit dataChanged(this);
            QTimer::singleShot(0, this, SLOT(moveToEnd()));
        }
    } else {
        logger->info("doTurn not possible!");
        QTimer::singleShot(0, this, SLOT(moveToEnd()));
    }

}


void TaskXsensNavigation::moveToEnd(){
    if(this->isEnabled()){
        logger->debug("Move to end");
        addData("state", "Move to end");
        emit dataChanged(this);
        emit newState("Move to end");
        distanceToTarget = this->navi->getDistance(this->getSettingsValue("targetNavigation").toString());
        if(distanceToTarget > this->getSettingsValue("targetTolerance").toDouble()){
            addData("remaining distance", distanceToTarget);
            emit dataChanged(this);
            moveToEndTimer.singleShot(this->getSettingsValue("signalTimer").toInt(),this, SLOT(moveToEnd()));
        } else {
            QTimer::singleShot(0, this, SLOT(controlNextState()));
        }
    } else {
        logger->info("moveToEnd not possible!");
        QTimer::singleShot(0, this, SLOT(emergencyStop()));
    }
}

void TaskXsensNavigation::controlNextState(){
    if(this->isEnabled()){
        logger->debug("idle, controlNextState");
        addData("state", "Idle, control next state");
        emit newState("Idle, control next state");
        emit dataChanged(this);

        if(this->getSettingsValue("loopActivated").toBool()){
            logger->debug("start again");
            QTimer::singleShot(0, this, SLOT(moveToStart()));
        } else {
            // Task finished, stop
            QTimer::singleShot(0, this, SLOT(stop()));
        }
    } else {
        logger->info("controlNextState not possible!");
        QTimer::singleShot(0, this, SLOT(emergencyStop())); 
    }
}


void TaskXsensNavigation::stop(){
    if(this->isEnabled()){
        running = false;
        logger->info("Taskxsensnavigation stopped");

        if (this->isActive())
        {
            this->taskTimer.stop();
            this->navi->setEnabled(false);
            this->xsensfollow->setEnabled(false);
            QTimer::singleShot(0, xsensfollow, SLOT(stop()));
            this->setEnabled(false);
            emit finished(this,true);
        }
    }
}

void TaskXsensNavigation::timeoutStop(){
    if(this->isEnabled()){
        running = false;
        logger->info("Taskxsensnavigation timeout stopped");

        if (this->isActive())
        {
            this->taskTimer.stop();
            this->navi->setEnabled(false);
            this->xsensfollow->setEnabled(false);
            QTimer::singleShot(0, xsensfollow, SLOT(stop()));

            this->setEnabled(false);
            emit finished(this,false);
        }
    }
}

void TaskXsensNavigation::emergencyStop(){
    if (this->isEnabled() == false){
        logger->info("Not enabled!");
        return;
    }
    running = false;
    logger->info( "Taskxsensnavigation emergency stopped" );

    if (this->isActive())
    {
        this->taskTimer.stop();
        this->navi->setEnabled(false);
        this->xsensfollow->setEnabled(false);
        QTimer::singleShot(0, xsensfollow, SLOT(stop()));
        this->setEnabled(false);
        emit finished(this,false);
    }
}


void TaskXsensNavigation::terminate()
{
    this->stop();
    RobotModule::terminate();
}

QWidget* TaskXsensNavigation::createView(QWidget *parent)
{
    return new TaskXsensNavigationForm(this, parent);
}

QList<RobotModule*> TaskXsensNavigation::getDependencies()
{
    QList<RobotModule*> ret;
    ret.append(xsensfollow);
    ret.append(sim);
    return ret;
}

void TaskXsensNavigation::controlEnabledChanged(bool b){
    if(b == false){
        logger->info("No longer enabled!");
        QTimer::singleShot(0, this, SLOT(emergencyStop()));
    }
}

