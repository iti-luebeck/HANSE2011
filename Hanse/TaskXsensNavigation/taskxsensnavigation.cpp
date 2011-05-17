/*
    Fahre zu Position A
    Mache x milisekunden Xsens-Following, dann einen Turn
    Fahre zu Position B
    Mache Turn 180
    Fahre zurück zu Position A
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


    connect(this, SIGNAL(stopSignal()), this, SLOT(stop()));

    connect(this, SIGNAL(moveToStartSignal()), this, SLOT(moveToStartSlot()));
    connect(this, SIGNAL(moveToBSignal()), this, SLOT(moveToBSlot()));
    connect(this, SIGNAL(moveToEndSignal()), this, SLOT(moveToEndSlot()));
    connect(this, SIGNAL(doTurnSignal()), this, SLOT(doTurnSlot()));
    connect(this, SIGNAL(doXsensFollowSignal()), this, SLOT(doXsensFollowSlot()));
    connect(this, SIGNAL(controlNextStateSignal()), this, SLOT(controlNextStateSlot()));

    taskTimer.moveToThread(this);
    moveToStartTimer.moveToThread(this);
    moveToEndTimer.moveToThread(this);
    doXsensFollowTimer.moveToThread(this);
    controlNextStateTimer.moveToThread(this);
}

bool TaskXsensNavigation::isActive(){
    return isEnabled();
}


void TaskXsensNavigation::init(){
    logger->debug("taskxsensnavigation init");
}



void TaskXsensNavigation::startBehaviour(){
    this->reset();
    logger->info("Taskxsensnavigation started" );
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
    addData("state", "start TaskXsensNavigation");
    emit dataChanged(this);

    emit updateSettings();

    if(this->getSettingsValue("timerActivated").toBool()){
        logger->debug("Taskxsensnavigation with timer stop");
        taskTimer.singleShot(this->getSettingsValue("taskStopTime").toInt(),this, SLOT(timeoutStop()));
        taskTimer.start();
    }

    emit moveToStartSignal();

}

void TaskXsensNavigation::moveToStartSlot(){
    if(this->isEnabled()){
        logger->debug("move to start");
        addData("state", "Move to start");
        emit dataChanged(this);

        // First navigate to start position
        distanceToStart = this->navi->getDistance(this->getSettingsValue("startNavigation").toString());
        if(distanceToStart > this->getSettingsValue("startTolerance").toDouble()){
            addData("remaining distance", distanceToStart);
            emit dataChanged(this);
            moveToStartTimer.singleShot(this->getSettingsValue("signalTimer").toInt(),this, SLOT(moveToStartSlot()));
        } else {
            addData("remaining distance", "-");
            emit dataChanged(this);
            addData("state", "Startposition reached");
            emit dataChanged(this);

            emit doXsensFollowSignal();
        }
    }
}
void TaskXsensNavigation::doXsensFollowSlot(){
    if(this->isEnabled()){
        logger->debug("Do xsensfollowing");
        addData("state", "Do xsensfollowing");
        emit dataChanged(this);

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
        QTimer::singleShot(tempWait, this, SLOT(finishXsensFollowSlot()));
    }
}

void TaskXsensNavigation::finishXsensFollowSlot(){
    if(this->isEnabled()){ this->xsensfollow->setSettingsValue("turnClockwise", false);
        QTimer::singleShot(0, xsensfollow, SLOT(stop()));
        QTimer::singleShot(0, this, SLOT(moveToBSlot()));
    }
}


void TaskXsensNavigation::moveToBSlot(){
    if(this->isEnabled()){
        logger->debug("move to b");
        addData("state", "Move to B");
        emit dataChanged(this);

        // Navigate to B position
        distanceToB = this->navi->getDistance(this->getSettingsValue("bNavigation").toString());
        if(distanceToB > this->getSettingsValue("bTolerance").toDouble()){
            addData("remaining distance", distanceToB);
            emit dataChanged(this);
            moveToBTimer.singleShot(this->getSettingsValue("signalTimer").toInt(),this, SLOT(moveToBSlot()));
        } else {
            addData("remaining distance", "-");
            emit dataChanged(this);
            addData("state", "B-Position reached");
            emit dataChanged(this);

            emit doTurnSignal();
        }
    }
}


void TaskXsensNavigation::doTurnSlot(){
    if(this->isEnabled()){
        logger->debug("turn180");
        addData("state", "Turn180");
        emit dataChanged(this);

        if(!this->turn180->isEnabled()){
            logger->debug("enable turn180");
            this->turn180->setSettingsValue("hysteresis", this->getSettingsValue("hysteresis").toInt());
            this->turn180->setSettingsValue("p", this->getSettingsValue("p").toInt());
            QTimer::singleShot(0, turn180, SLOT(startBehaviour()));
        }

        // Do Turn180. If its finished, it will be disabled
        if(this->turn180->isEnabled()){
            doTurnTimer.singleShot(this->getSettingsValue("signalTimer").toInt(),this, SLOT(doTurnSlot()));
        } else {
            addData("state", "Turn180 finished");
            emit dataChanged(this);
            emit moveToEndSlot();
        }
    }

}


void TaskXsensNavigation::moveToEndSlot(){
    if(this->isEnabled()){
        logger->debug("Move to end");
        addData("state", "Move to end");
        emit dataChanged(this);

        distanceToTarget = this->navi->getDistance(this->getSettingsValue("targetNavigation").toString());
        if(distanceToTarget > this->getSettingsValue("targetTolerance").toDouble()){
            addData("remaining distance", distanceToTarget);
            emit dataChanged(this);
            moveToEndTimer.singleShot(this->getSettingsValue("signalTimer").toInt(),this, SLOT(moveToEndSlot()));
        } else {
            emit controlNextStateSignal();
        }
    }
}

void TaskXsensNavigation::controlNextStateSlot(){
    if(this->isEnabled()){
        logger->debug("idle, controlNextState");
        addData("state", "Idle, control next state");
        emit dataChanged(this);

        if(this->getSettingsValue("loopActivated").toBool()){
            logger->debug("start again");
            emit moveToStartSignal();
        } else {
            // Task finished, stop
            emit stopSignal();
        }
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
            // this->wall->echo->setEnabled(false);
            QTimer::singleShot(0, xsensfollow, SLOT(stop()));

            this->setEnabled(false);
            emit finished(this,true);
        }
    }
}

void TaskXsensNavigation::timeoutStop(){
    if(this->isEnabled()){running = false;
        logger->info("Taskxsensnavigation timeout stopped");

        if (this->isActive())
        {
            this->taskTimer.stop();

            this->navi->setEnabled(false);
            //this->wall->echo->setEnabled(false);
            QTimer::singleShot(0, xsensfollow, SLOT(stop()));

            this->setEnabled(false);
            emit finished(this,false);
        }
    }
}

void TaskXsensNavigation::emergencyStop(){
    running = false;
    logger->info( "Taskxsensnavigation emergency stopped" );

    if (this->isActive())
    {
        this->taskTimer.stop();
        this->navi->setEnabled(false);
        // this->wall->echo->setEnabled(false);
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


