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
#include <Behaviour_XsensFollowing/behaviour_xsensfollowing.h>
#include <Module_Navigation/module_navigation.h>
#include <Behaviour_TurnOneEighty/behaviour_turnoneeighty.h>

TaskXsensNavigation::TaskXsensNavigation(QString id, Module_Simulation *sim, Behaviour_XsensFollowing *xf, Module_Navigation *n, Behaviour_TurnOneEighty *o180)
    : RobotBehaviour(id)
{
    this->sim = sim;
    this->xsensfollow = xf;
    this->navi = n;
    this->turn180 = o180;
}

bool TaskXsensNavigation::isActive(){
    return active;
}


void TaskXsensNavigation::init(){
    logger->debug("taskxsensnavigation init");
    active = false;
    setEnabled(false);
    // Default task settings
    this->setDefaultValue("description", "");
    this->setDefaultValue("taskStopTime",120000);
    this->setDefaultValue("signalTimer",1000);

    this->setDefaultValue("startNavigation", "a");
    this->setDefaultValue("startTolerance", 2);
    this->setDefaultValue("bNavigation", "b");
    this->setDefaultValue("bTolerance", 2);
    this->setDefaultValue("targetNavigation", "c");
    this->setDefaultValue("targetTolerance", 2);

    this->setDefaultValue("timerActivated", true);
    this->setDefaultValue("loopActivated", true);

    // Default xsensfollow settings
    this->setDefaultValue("ffSpeed", 0.5);
    this->setDefaultValue("kp", 0.4);
    this->setDefaultValue("delta", 10);
    this->setDefaultValue("timer", 30);
    this->setDefaultValue("driveTime", 10000);
    this->setDefaultValue("waitTime", 10000);

    // Default turn180 settings
    this->setDefaultValue("hysteresis", 10);
    this->setDefaultValue("p", 0.4);
    this->setDefaultValue("degree", 180);

    taskTimer.setSingleShot(true);
    taskTimer.moveToThread(this);

    connect(this, SIGNAL(enabled(bool)), this, SLOT(controlEnabledChanged(bool)));
    connect(navi, SIGNAL(reachedWaypoint(QString)), this, SLOT(reachedWaypoint(QString)));
    connect(turn180, SIGNAL(turn180finished()), this, SLOT(turn180Finished()));

    connect(this, SIGNAL(setNewWaypoint(QString)), navi, SLOT(gotoWayPoint(QString)));
}

void TaskXsensNavigation::startBehaviour(){
    if (isActive()){
        logger->info("Already active!");
        return;
    }

    this->reset();
    logger->info("Taskxsensnavigation started" );
    setHealthToOk();

    // Stateoverview
    emit newStateOverview("Move to start");
    emit newStateOverview("Do xsensfollowing");
    emit newStateOverview("Move to B");
    emit newStateOverview("Turn180");
    emit newStateOverview("Move to end");
    if(this->getSettingsValue("loopActivated").toBool()){
        emit newStateOverview("...in a loop");
    }

    active = true;
    if(!isEnabled()){
        setEnabled(true);
    }
    emit started(this);

    // Enable all components
    if(!this->navi->isEnabled()) {
        this->navi->setEnabled(true);
    }

    state = XSENS_NAV_STATE_MOVE_START;
    addData("loop", this->getSettingsValue("loopActivated").toBool());
    addData("timer", this->getSettingsValue("timerActivated").toBool());
    addData("state", state);
    emit dataChanged(this);

    emit updateSettings();

    if(this->getSettingsValue("timerActivated").toBool()){
        logger->debug("Taskxsensnavigation with timer stop");
        taskTimer.singleShot(this->getSettingsValue("taskStopTime").toInt(),this, SLOT(timeoutStop()));
        taskTimer.start();
    }

    taskTimer.singleShot(0, this, SLOT(stateChanged()));
}

void TaskXsensNavigation::stateChanged()
{
    if(!isActive()){
        return;
    }

    if (state == XSENS_NAV_STATE_MOVE_START) {

        if (this->navi->getHealthStatus().isHealthOk()) {
            logger->debug("move to start");
            emit setNewWaypoint(this->getSettingsValue("startNavigation").toString());
        } else {
            logger->info("move to start not possible!");
            emergencyStop();
        }

    } else if (state == XSENS_NAV_STATE_FOLLOW_XSENS) {

        if (this->xsensfollow->getHealthStatus().isHealthOk()) {
            logger->debug("xsens following");
            initBehaviourParameters();

            if (!this->xsensfollow->isEnabled()) {
                logger->debug("enable xsensfollow");
                QTimer::singleShot(0, xsensfollow, SLOT(startBehaviour()));
            } else {
                QTimer::singleShot(0, xsensfollow, SLOT(reset()));
            }

            // Finish state after drivetime msec and a little bit time to finish turn
            int tempWait = this->getSettingsValue("driveTime").toInt() + this->getSettingsValue("waitTime").toInt();
            QTimer::singleShot(tempWait, this, SLOT(xsensFollowFinished()));
        } else {
            logger->info("xsens following not possible!");
            state = XSENS_NAV_STATE_MOVE_B;
            QTimer::singleShot(0, this, SLOT(stateChanged()));
        }

    } else if (state == XSENS_NAV_STATE_MOVE_B) {

        if (this->navi->getHealthStatus().isHealthOk()) {
            logger->debug("move to B");
            emit setNewWaypoint(this->getSettingsValue("bNavigation").toString());
        } else {
            logger->info("move to B not possible!");
            emergencyStop();
        }

    } else if (state == XSENS_NAV_STATE_TURN_180) {

        if(this->turn180->getHealthStatus().isHealthOk()) {
            logger->debug("turn 180 degrees");
            initBehaviourParameters();

            if (!this->turn180->isEnabled()){
                logger->debug("enable turn180");
                QTimer::singleShot(0, turn180, SLOT(startBehaviour()));
            } else {
                QTimer::singleShot(0, turn180, SLOT(reset()));
            }
        } else {
            logger->info("turn 180 degrees not possible!");
            state = XSENS_NAV_STATE_MOVE_END;
            QTimer::singleShot(0, this, SLOT(stateChanged()));
        }

    } else if (state == XSENS_NAV_STATE_MOVE_END) {

        if (this->navi->getHealthStatus().isHealthOk()) {
            logger->debug("move to end");
            emit setNewWaypoint(this->getSettingsValue("targetNavigation").toString());
        } else {
            logger->info("move to end not possible!");
            emergencyStop();
        }

    } else if (state == XSENS_NAV_STATE_DONE) {

        logger->debug("idle, controlNextState");

        if(this->getSettingsValue("loopActivated").toBool()){
            logger->debug("start again");
            state = XSENS_NAV_STATE_MOVE_START;
            QTimer::singleShot(0, this, SLOT(stateChanged()));
        } else {
            // Task finished, stop
            stop();
        }

    }

    addData("state", state);
    emit dataChanged(this);
    QString navstate = this->navi->getDataValue("state").toString();
    QString navsubstate = this->navi->getDataValue("substate").toString();
    QString stateComment = state + " - "+ navstate + " - " + navsubstate;
    emit newState(this->getId(),stateComment);
}

void TaskXsensNavigation::initBehaviourParameters()
{
    if (state == XSENS_NAV_STATE_FOLLOW_XSENS) {

        this->xsensfollow->setSettingsValue("ffSpeed", this->getSettingsValue("ffSpeed").toString());
        this->xsensfollow->setSettingsValue("kp", this->getSettingsValue("kp").toString());
        this->xsensfollow->setSettingsValue("delta", this->getSettingsValue("delta").toString());
        this->xsensfollow->setSettingsValue("timer", this->getSettingsValue("timer").toString());

        this->xsensfollow->setSettingsValue("turnClockwise", true);
        this->xsensfollow->setSettingsValue("driveTime", this->getSettingsValue("driveTime").toString());

    } else if (state == XSENS_NAV_STATE_TURN_180) {

        this->turn180->setSettingsValue("hysteresis", this->getSettingsValue("hysteresis").toFloat());
        this->turn180->setSettingsValue("p", this->getSettingsValue("p").toFloat());
        this->turn180->setSettingsValue("degree", this->getSettingsValue("degree").toFloat());

    }
}

void TaskXsensNavigation::reachedWaypoint(QString)
{
    if (state == XSENS_NAV_STATE_MOVE_START) {
        state = XSENS_NAV_STATE_FOLLOW_XSENS;
    } else if (state == XSENS_NAV_STATE_MOVE_B) {
        state = XSENS_NAV_STATE_TURN_180;
    } else if (state == XSENS_NAV_STATE_MOVE_END) {
        state = XSENS_NAV_STATE_DONE;
    }

    taskTimer.singleShot(0, this, SLOT(stateChanged()));
}

void TaskXsensNavigation::xsensFollowFinished()
{
    QTimer::singleShot(0, xsensfollow, SLOT(stop()));
    state = XSENS_NAV_STATE_MOVE_B;
    taskTimer.singleShot(0, this, SLOT(stateChanged()));
}

void TaskXsensNavigation::turn180Finished()
{
    QTimer::singleShot(0, turn180, SLOT(stop()));
    state = XSENS_NAV_STATE_MOVE_END;
    taskTimer.singleShot(0, this, SLOT(stateChanged()));
}

void TaskXsensNavigation::stop(){
    if(!isActive()){
        logger->info("Not active!");
        return;
    }

    logger->info("Taskxsensnavigation stopped");
    active = false;
    setEnabled(false);

    this->taskTimer.stop();
    this->navi->setEnabled(false);
    this->xsensfollow->setEnabled(false);
    QTimer::singleShot(0, xsensfollow, SLOT(stop()));

    emit finished(this,true);
}

void TaskXsensNavigation::timeoutStop(){
    if(!isActive()){
        return;
    }

    logger->info("Taskxsensnavigation timeout stopped");
    active = false;
    setEnabled(false);

    this->taskTimer.stop();
    this->navi->setEnabled(false);
    this->xsensfollow->setEnabled(false);
    QTimer::singleShot(0, xsensfollow, SLOT(stop()));

    emit finished(this,false);

}

void TaskXsensNavigation::emergencyStop(){
    if (!isActive()){
        logger->info("Not active, no emergency stop needed");
        return;
    }

    logger->info("Taskxsensnavigation emergency stopped");
    active = false;
    setEnabled(false);

    this->taskTimer.stop();
    this->navi->setEnabled(false);
    this->xsensfollow->setEnabled(false);
    QTimer::singleShot(0, xsensfollow, SLOT(stop()));

    emit finished(this,false);

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

void TaskXsensNavigation::controlEnabledChanged(bool enabled){
    if(!enabled && isActive()){
        logger->info("Disable and deactivate TaskXsensNavigation");
        stop();
    } else if(!enabled && !isActive()){
        //logger->info("Still deactivated");
    } else if(enabled && !isActive()){
        //logger->info("Enable and activate TaskXsensNavigation");
        //startBehaviour();
    } else {
        //logger->info("Still activated");
    }
}

