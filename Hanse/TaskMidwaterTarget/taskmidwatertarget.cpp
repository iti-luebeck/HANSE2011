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

}

bool TaskMidwaterTarget::isActive(){
    return active;
}


void TaskMidwaterTarget::init(){
    logger->debug("taskmidwatertarget init");

    this->setDefaultValue("taskStopTime",120000);
    this->setDefaultValue("timerActivated", true);

    this->setDefaultValue("taskStartPoint", "SP");
    this->setDefaultValue("finishPoint", "tmtF");

    active = false;
    setEnabled(false);
    connect(this, SIGNAL(enabled(bool)), this, SLOT(controlEnabledChanged(bool)));
    connect(navi, SIGNAL(reachedWaypoint(QString)), this, SLOT(controlFinishedWaypoints(QString)));
    connect(xsensfollow, SIGNAL(newXsensState(QString)), this, SLOT(controlXsensState(QString)));
    connect(ballfollow, SIGNAL(newBallState(QString)), this, SLOT(controlBallState(QString)));


}

void TaskMidwaterTarget::reset() {
    RobotModule::reset();

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

    emit newStateOverview(TASK_STATE_MOVE_TO_TASK_START);
    emit newStateOverview(TASK_STATE_BALLFOLLOWING);
    emit newStateOverview(TASK_STATE_XSENSFOLLOW);
    emit newStateOverview(TASK_STATE_FINISH_INSPECTION);


    taskState = TASK_STATE_START;
    showTaskState();

    emit updateSettings();

    if(this->getSettingsValue("timerActivated").toBool()){
        logger->info("TaskMidwaterTarget with timer stop");
        taskTimer.singleShot(this->getSettingsValue("taskStopTime").toInt(),this, SLOT(timeoutStop()));
        taskTimer.start();
    }

    taskState = TASK_STATE_MOVE_TO_TASK_START;
    controlTaskStates();
}


void TaskMidwaterTarget::controlTaskStates(){
    if(taskState == TASK_STATE_MOVE_TO_TASK_START){
        showTaskState();
        this->navi->gotoWayPoint(this->getSettingsValue("taskStartPoint").toString());

    } else if(taskState == TASK_STATE_BALLFOLLOWING){
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
            taskState = TASK_STATE_XSENSFOLLOW;
            controlTaskStates();
        }

    } else if(taskState == TASK_STATE_XSENSFOLLOW){

        Position currentPosition = this->navi->getCurrentPosition();
        Waypoint newWP;
        newWP.name = this->getSettingsValue("finishPoint").toString();
        newWP.posX = currentPosition.getX();
        newWP.posY = currentPosition.getY();
        newWP.depth = currentPosition.getZ();
        newWP.startAngle = 0.0;
        newWP.useStartAngle = false;
        newWP.exitAngle = currentPosition.getYaw();
        newWP.useExitAngle = true;
        this->navi->addWaypoint(this->getSettingsValue("finishPoint").toString(), newWP);

        initBehaviourParameters();
        showTaskState();
        if(this->xsensfollow->getHealthStatus().isHealthOk()){
            if (!this->xsensfollow->isActive()) {
                logger->info("Activate xsensfollowing");
                QTimer::singleShot(0, xsensfollow, SLOT(startBehaviour()));
            } else {
                QTimer::singleShot(0, xsensfollow, SLOT(reset()));
            }
        } else {
            logger->info("Xsensfollowing not possible");
            taskState = TASK_STATE_FINISH_INSPECTION;
            controlTaskStates();
        }

    } else if(taskState == TASK_STATE_FINISH_INSPECTION){
        showTaskState();
        this->navi->gotoWayPoint(this->getSettingsValue("finishPoint").toString());

    } else if(taskState == TASK_STATE_END){
        showTaskState();
        stop();
    }
}


void TaskMidwaterTarget::controlFinishedWaypoints(QString waypoint){
    if(!isActive()){
        return;
    }

    if(waypoint == this->getSettingsValue("taskStartPoint").toString()){
        logger->info(this->getSettingsValue("taskStartPoint").toString() +" reached");       
        taskState = TASK_STATE_BALLFOLLOWING;
        controlTaskStates();

    } else if(waypoint ==  this->getSettingsValue("adjustPoint").toString()){
        logger->info(this->getSettingsValue("adjustPoint").toString() +" reached");
        taskState = TASK_STATE_END;
        controlTaskStates();

     } else {
        // None of current task waypoints reached oO
        logger->info("Something is wrong with navigation waypoints, abort...");
        emergencyStop();;
    }

}

void TaskMidwaterTarget::controlBallState(QString newState){
    if(!isActive()){
        return;
    }

    if(newState == STATE_PASSED && taskState == TASK_STATE_BALLFOLLOWING){
        taskState = TASK_STATE_XSENSFOLLOW;
        QTimer::singleShot(0, ballfollow, SLOT(stop()));
        controlTaskStates();
    }
}


void TaskMidwaterTarget::controlXsensState(QString newState){
    if(!isActive()){
        return;
    }

    if(newState == STATE_FINISHED && taskState == TASK_STATE_XSENSFOLLOW){
        taskState = TASK_STATE_FINISH_INSPECTION;
        QTimer::singleShot(0, xsensfollow, SLOT(stop()));
        controlTaskStates();
    }
}

void TaskMidwaterTarget::initBehaviourParameters(){
    if(taskState == TASK_STATE_XSENSFOLLOW){
        this->xsensfollow->setSettingsValue("driveTime", 10000);
        this->xsensfollow->setSettingsValue("turnClockwise", false);
        this->xsensfollow->setSettingsValue("nuberTurns", 3);
        this->xsensfollow->setSettingsValue("enableTurn", true);
    } else if(taskState == TASK_STATE_BALLFOLLOWING){
        // Nothing
    }
}

void TaskMidwaterTarget::showTaskState(){
    logger->info(taskState);
    emit newState(taskState);
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

    taskState = TASK_STATE_END;
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

    taskState = TASK_STATE_END_FAILED;
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

    taskState = TASK_STATE_END_FAILED;
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

QString TaskMidwaterTarget::getTaskState(){
    return taskState;
}
