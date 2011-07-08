#include "taskmidwatertarget.h"
#include "taskmidwatertargetform.h"
#include <QtGui>
#include <Module_Simulation/module_simulation.h>
#include <Module_Navigation/module_navigation.h>
#include <Behaviour_BallFollowing/behaviour_ballfollowing.h>
#include <Behaviour_XsensFollowing/behaviour_xsensfollowing.h>


TaskMidwaterTarget::TaskMidwaterTarget(QString id, Module_Simulation *sim, Module_Navigation *n, Behaviour_BallFollowing *bf, Behaviour_XsensFollowing *xf)
    : RobotBehaviour(id)
{
    this->sim = sim;
    this->navi = n;
    this->xsensfollow = xf;
    this->ballfollow = bf;

    taskTimer.moveToThread(this);
    searchBallTimer.moveToThread(this);
}

bool TaskMidwaterTarget::isActive(){
    return active;
}


void TaskMidwaterTarget::init(){
    logger->debug("taskmidwatertarget init");

    this->setDefaultValue("taskStopTime",3);
    this->setDefaultValue("timerActivated", true);

    this->setDefaultValue("taskStartPoint", "mid start");
    this->setDefaultValue("target waypoint", "mid target");
    this->setDefaultValue("avoid waypoint left", "mid avoid left");
    this->setDefaultValue("avoid waypoint right", "mid avoid right");
    this->setDefaultValue("finishPoint", "mid finish");

    active = false;
    setEnabled(false);
    connect(this, SIGNAL(enabled(bool)), this, SLOT(controlEnabledChanged(bool)));
    connect(navi, SIGNAL(reachedWaypoint(QString)), this, SLOT(controlFinishedWaypoints(QString)));
    connect(xsensfollow, SIGNAL(newXsensState(QString)), this, SLOT(controlXsensState(QString)));
    connect(ballfollow, SIGNAL(newBallState(QString)), this, SLOT(controlBallState(QString)));

    reset();
}

void TaskMidwaterTarget::reset() {
    RobotModule::reset();

    this->tries = 0;
}


void TaskMidwaterTarget::startBehaviour(){
    if (isActive()){
        logger->info("Already active!");
        return;
    }

    this->reset();
    setHealthToOk();



    active = true;
    if(!isEnabled()){
        setEnabled(true);
    }
    emit started(this);


    // Enable all components
    if(!this->navi->isEnabled()){
        this->navi->setEnabled(true);
    }

    emit newStateOverview(MIDWATER_STATE_MOVE_TO_TASK_START);
    emit newStateOverview(MIDWATER_STATE_FIND_BALL);
    emit newStateOverview(MIDWATER_STATE_AVOID_BALL);
    emit newStateOverview(MIDWATER_STATE_BALLFOLLOWING);
    emit newStateOverview(MIDWATER_STATE_XSENSFOLLOW);
    emit newStateOverview(MIDWATER_STATE_MOVE_INSPECT);
    emit newStateOverview(MIDWATER_STATE_INSPECT);
    emit newStateOverview(MIDWATER_STATE_XSENSFOLLOW);


    taskState = MIDWATER_STATE_START;
    showTaskState();

    emit updateSettings();

    if(this->getSettingsValue("timerActivated").toBool()){
        logger->info("TaskMidwaterTarget with timer stop");
        int temp = this->getSettingsValue("taskStopTime").toInt()*60000;
        taskTimer.singleShot(temp,this, SLOT(timeoutStop()));
        addData("task stop time", temp);
        taskTimer.start();
    }

    taskState = MIDWATER_STATE_MOVE_TO_TASK_START;
    controlTaskStates();
}


void TaskMidwaterTarget::controlTaskStates(){
    if (taskState == MIDWATER_STATE_MOVE_TO_TASK_START) {

        showTaskState();
        this->navi->gotoWayPoint(this->getSettingsValue("taskStartPoint").toString());

    } else if (taskState == MIDWATER_STATE_FIND_BALL) {

        showTaskState();
        if(this->ballfollow->getHealthStatus().isHealthOk()){
            if (!this->ballfollow->isActive()) {
                logger->info("Activate ballfollowing");
                QTimer::singleShot(0, ballfollow, SLOT(startBehaviour()));
            } else {
                QTimer::singleShot(0, ballfollow, SLOT(reset()));
            }
        } else {
            logger->info("Ballfollowing not possible");
            taskState = MIDWATER_STATE_XSENSFOLLOW;
            controlTaskStates();
        }

    } else if (taskState == MIDWATER_STATE_AVOID_BALL) {

        showTaskState();
        this->navi->gotoWayPoint(this->getSettingsValue("avoid waypoint right").toString());

    } else if(taskState == MIDWATER_STATE_BALLFOLLOWING){
        showTaskState();
        if(this->ballfollow->getHealthStatus().isHealthOk()){
            if (!this->ballfollow->isActive()) {
                logger->info("Activate ballfollowing");
                QTimer::singleShot(0, ballfollow, SLOT(startBehaviour()));
            } else {
                QTimer::singleShot(0, ballfollow, SLOT(reset()));
            }
        } else {
            logger->info("Ballfollowing not possible");
            taskState = MIDWATER_STATE_XSENSFOLLOW;
            controlTaskStates();
        }

    } else if(taskState == MIDWATER_STATE_XSENSFOLLOW){

        initBehaviourParameters();
        showTaskState();
        if(this->xsensfollow->getHealthStatus().isHealthOk()){
            if (!this->xsensfollow->isActive()) {
                logger->info("Activate xsensfollowing");
                QTimer::singleShot(0, xsensfollow, SLOT(startBehaviour()));
            } else {
                QTimer::singleShot(0, xsensfollow, SLOT(reset()));
            }

            searchBallTimer.singleShot(15000, this, SLOT(cutFinished()));
        } else {
            logger->info("Xsensfollowing not possible");
            taskState = MIDWATER_STATE_MOVE_INSPECT;
            controlTaskStates();
        }

    } else if (taskState == MIDWATER_STATE_MOVE_INSPECT) {

        showTaskState();
        this->navi->gotoWayPoint(this->getSettingsValue("target waypoint").toString());

    } else if (taskState == MIDWATER_STATE_INSPECT) {

        showTaskState();
        if (!this->ballfollow->isActive()) {
            logger->info("Activate ballfollowing");
            QTimer::singleShot(0, ballfollow, SLOT(startBehaviour()));
        } else {
            QTimer::singleShot(0, ballfollow, SLOT(reset()));
        }
        searchBallTimer.singleShot(30000, this, SLOT(ballNotFound()));

    } else if(taskState == MIDWATER_STATE_END){

        showTaskState();
        stop();

    }
}


void TaskMidwaterTarget::controlFinishedWaypoints(QString waypoint) {

    if(!isActive()){
        return;
    }

    if (taskState == MIDWATER_STATE_MOVE_TO_TASK_START) {
        if (waypoint == this->getSettingsValue("taskStartPoint").toString()) {
            logger->info(this->getSettingsValue("taskStartPoint").toString() +" reached");
            taskState = MIDWATER_STATE_FIND_BALL;
            controlTaskStates();
        } else {
            navi->gotoWayPoint(this->getSettingsValue("taskStartPoint").toString());
        }
    }

    if (taskState == MIDWATER_STATE_AVOID_BALL) {
        if (waypoint == this->getSettingsValue("avoid waypoint right").toString()) {
            navi->gotoWayPoint(this->getSettingsValue("avoid waypoint left").toString());
        } else if (waypoint == this->getSettingsValue("avoid waypoint left").toString()) {
            navi->gotoWayPoint(this->getSettingsValue("target waypoint").toString());
        } else if (waypoint == this->getSettingsValue("target waypoint").toString()) {
            logger->info("avoid midwater target success");
            taskState = MIDWATER_STATE_BALLFOLLOWING;
            controlTaskStates();
        } else {
            navi->gotoWayPoint(this->getSettingsValue("avoid waypoint right").toString());
        }
    }

    if (taskState == MIDWATER_STATE_MOVE_INSPECT) {
        taskState = MIDWATER_STATE_INSPECT;
        controlTaskStates();
    }

}

void TaskMidwaterTarget::controlBallState(QString newState){

    if (!isActive()) {
        return;
    }

    if (taskState == MIDWATER_STATE_FIND_BALL) {
        if (newState == BALL_STATE_FOUND_BALL) {
            QTimer::singleShot(0, ballfollow, SLOT(stop()));
            searchBallTimer.singleShot(10000, this, SLOT(markBallPosition()));
        }
    }

    if (taskState == MIDWATER_STATE_BALLFOLLOWING) {
        if (newState == BALL_STATE_FOUND_BALL) {
            ballfollow->setState(BALL_STATE_CUT_BALL);
        } else if (newState == BALL_STATE_DO_CUT) {
            QTimer::singleShot(0, ballfollow, SLOT(stop()));
            taskState = MIDWATER_STATE_XSENSFOLLOW;
            controlTaskStates();
        }
    }

    if (taskState == MIDWATER_STATE_INSPECT) {
        if (newState == BALL_STATE_FOUND_BALL) {
            tries++;

            if (tries > 2) {
                searchBallTimer.stop();
                logger->info("failed to cut ball 2 times, proceeding with next task");
                QTimer::singleShot(0, ballfollow, SLOT(stop()));
                taskState = MIDWATER_STATE_END;
                controlTaskStates();
            } else {
                searchBallTimer.stop();
                ballfollow->setState(BALL_STATE_CUT_BALL);
                taskState = MIDWATER_STATE_BALLFOLLOWING;
            }
        }
    }
}


void TaskMidwaterTarget::controlXsensState(QString newState){
    if(!isActive()){
        return;
    }

    if(newState == STATE_FINISHED && taskState == MIDWATER_STATE_XSENSFOLLOW){
        taskState = MIDWATER_STATE_MOVE_INSPECT;
        QTimer::singleShot(0, xsensfollow, SLOT(stop()));
        controlTaskStates();
    }
}

void TaskMidwaterTarget::initBehaviourParameters(){
    if(taskState == MIDWATER_STATE_XSENSFOLLOW){
        this->xsensfollow->setSettingsValue("driveTime", 15000);
        this->xsensfollow->setSettingsValue("turnClockwise", false);
        this->xsensfollow->setSettingsValue("numberTurns", 0);
        this->xsensfollow->setSettingsValue("enableTurn", false);
    } else if(taskState == MIDWATER_STATE_BALLFOLLOWING){
        // Nothing
    }
}

void TaskMidwaterTarget::showTaskState(){
    logger->info(taskState);
    emit newState(this->getId(), taskState);
    addData("taskState", taskState);
    emit dataChanged(this);
}


void TaskMidwaterTarget::stop(){
    if(!isActive()){
        logger->info("Not active!");
        return;
    }

    logger->info("TaskMidwaterTarget stopped" );

    active = false;
    setEnabled(false);

    taskState = MIDWATER_STATE_END;
    showTaskState();

    if(navi->hasGoal()){
        QTimer::singleShot(0, navi, SLOT(clearGoal()));
    }

    taskTimer.stop();
    emit finished(this,true);
}

void TaskMidwaterTarget::timeoutStop(){
    if(!isActive()){
        logger->info("Not active!");
        return;
    }

    logger->info("TaskMidwaterTarget timeout stopped" );

    active = false;
    setEnabled(false);

    taskState = MIDWATER_STATE_END_FAILED;
    showTaskState();

    if(navi->hasGoal()){
        QTimer::singleShot(0, navi, SLOT(clearGoal()));
    }

    taskTimer.stop();
    emit finished(this,false);

}



void TaskMidwaterTarget::emergencyStop(){
    if(!isActive()){
        logger->info("Not active, no emergency stop needed");
        return;
    }

    logger->info("TaskMidwaterTarget emergency stopped" );

    active = false;
    setEnabled(false);

    taskState = MIDWATER_STATE_END_FAILED;
    showTaskState();

    if(navi->hasGoal()){
        QTimer::singleShot(0, navi, SLOT(clearGoal()));
    }

    taskTimer.stop();
    emit finished(this,false);

}


void TaskMidwaterTarget::terminate()
{
    this->stop();
    RobotModule::terminate();
}

QWidget* TaskMidwaterTarget::createView(QWidget *parent)
{
    return new TaskMidwaterTargetForm(this, parent);
}

QList<RobotModule*> TaskMidwaterTarget::getDependencies()
{
    QList<RobotModule*> ret;
    ret.append(navi);
    ret.append(sim);
    return ret;
}

void TaskMidwaterTarget::controlEnabledChanged(bool enabled){
    if(!enabled && isActive()){
        logger->info("Disable and deactivate TaskMidwaterTarget");
        stop();
    } else if(!enabled && !isActive()){
        //logger->info("Still deactivated");
    } else if(enabled && !isActive()){
        //logger->info("Enable and activate TaskMidwaterTarget");
        //startBehaviour();
    } else {
        //logger->info("Still activated");
    }
}

QString TaskMidwaterTarget::getTaskState()
{
    return taskState;
}

void TaskMidwaterTarget::markBallPosition()
{
    Waypoint targetWaypoint(this->getSettingsValue("target waypoint").toString(),
                            this->navi->getCurrentPosition().getX(),
                            this->navi->getCurrentPosition().getY(),
                            1.7,
                            false,
                            0.0,
                            true,
                            this->navi->getCurrentPosition().getYaw());
    this->navi->addWaypoint(this->getSettingsValue("target waypoint").toString(), targetWaypoint);
    taskState = MIDWATER_STATE_AVOID_BALL;
    controlTaskStates();
}

void TaskMidwaterTarget::cutFinished()
{
    QTimer::singleShot(0, xsensfollow, SLOT(stop()));
    taskState = MIDWATER_STATE_MOVE_INSPECT;
    controlTaskStates();
}

void TaskMidwaterTarget::ballNotFound()
{
    QTimer::singleShot(0, ballfollow, SLOT(stop()));
    taskState = MIDWATER_STATE_END;
    controlTaskStates();
}
