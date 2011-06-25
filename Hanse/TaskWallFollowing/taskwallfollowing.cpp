#include "taskwallfollowing.h"
#include "taskwallfollowingform.h"
#include <QtGui>
#include <Module_Simulation/module_simulation.h>
#include <Module_Navigation/module_navigation.h>
#include <Behaviour_WallFollowing/behaviour_wallfollowing.h>
#include <Behaviour_TurnOneEighty/behaviour_turnoneeighty.h>

TaskWallFollowing::TaskWallFollowing(QString id, Behaviour_WallFollowing *w, Module_Simulation *sim, Module_Navigation *n, Behaviour_TurnOneEighty *o180)
    : RobotBehaviour(id)
{
    this->sim = sim;
    this->wall = w;
    this->turn180 = o180;
    this->navi = n;


}

bool TaskWallFollowing::isActive(){
    return active;
}


void TaskWallFollowing::init(){
    logger->debug("taskwallfollowing init");
    active = false;
    setEnabled(false);
    // Default task settings
    this->setDefaultValue("taskStopTime",120000);
    this->setDefaultValue("signalTimer",1000);
    this->setDefaultValue("timerActivated", true);

    // Default navigation settings
    this->setDefaultValue("taskStartPoint", "SP");
    this->setDefaultValue("wallAdjustPoint", "AP");
    this->setDefaultValue("useAdjustPoint", true);
    this->setDefaultValue("goal1point1", "go1p2");
    this->setDefaultValue("goal1point2", "go2p1");
    this->setDefaultValue("apDist" , 1.5);

    // Default turn180 settings
    this->setDefaultValue("hysteresis", 10);
    this->setDefaultValue("p", 0.4);
    this->setDefaultValue("degree", 180);

    taskStopTimer.setSingleShot(true);
    taskStopTimer.moveToThread(this);
    calcTimer.moveToThread(this);

    connect(this, SIGNAL(enabled(bool)), this, SLOT(controlEnabledChanged(bool)));
    connect(navi, SIGNAL(reachedWaypoint(QString)), this, SLOT(controlFinishedWaypoints(QString)));
    connect(turn180, SIGNAL(turn180finished(QString)), this, SLOT(controlFinishedWaypoints(QString)));
    connect(&calcTimer,SIGNAL(timeout()),this,SLOT(controlAngleDistance()));
}

void TaskWallFollowing::reset() {
    RobotModule::reset();

    taskState = TASK_STATE_START;
}


void TaskWallFollowing::startBehaviour(){
    if (isActive()){
        logger->info("Already active!");
        return;
    }

    reset();
    setHealthToOk();

    emit updateSettings();
    calcTimer.start(100);

    active = true;
    if(!isEnabled()){
        setEnabled(true);
    }
    emit started(this);

    if(!this->navi->isEnabled()){
        logger->info("Activate navigation");
        this->navi->setEnabled(true);
    }

    logger->info(taskState);
    addData("taskState", taskState);
    emit newState(taskState);
    emit dataChanged(this);


    emit newStateOverview(TASK_STATE_MOVE_TO_TASK_START);
    emit newStateOverview(TASK_STATE_WALLFOLLOW_PART1);
    if(this->getSettingsValue("useAdjustPoint").toBool()){
        emit newStateOverview(TASK_STATE_ADJUST_HEADING);
        emit newStateOverview(TASK_STATE_WALLFOLLOW_PART2);
    }

    if(this->getSettingsValue("timerActivated").toBool()){
        logger->info("TaskWallFollowing with timer stop");
        taskStopTimer.singleShot(this->getSettingsValue("taskStopTime").toInt(),this, SLOT(timeoutStop()));
        taskStopTimer.start();
    }


    taskState = TASK_STATE_MOVE_TO_TASK_START;
    controlTaskStates();
}


void TaskWallFollowing::controlTaskStates(){
    if(!isActive()){
        return;
    }

    if(taskState == TASK_STATE_MOVE_TO_TASK_START){
        showTaskState();
        this->navi->gotoWayPoint(this->getSettingsValue("taskStartPoint").toString());

    } else if(taskState == TASK_STATE_WALLFOLLOW_PART1){
        showTaskState();
        if(this->wall->getHealthStatus().isHealthOk()){
            if (!this->wall->isActive()) {
                logger->info("Activate wallfollowing");
                QTimer::singleShot(0, wall, SLOT(startBehaviour()));
            } else {
                QTimer::singleShot(0, wall, SLOT(reset()));
            }
        } else {
            logger->info("Wallfollowing not possible");
            taskState = TASK_STATE_ADJUST_HEADING;
            controlTaskStates();
        }

    } else if(taskState == TASK_STATE_ADJUST_HEADING){
        showTaskState();
        this->navi->gotoWayPoint(this->getSettingsValue("wallAdjustPoint").toString());

    } else if(taskState == TASK_STATE_WALLFOLLOW_PART2){
        showTaskState();


    } else if(taskState == TASK_STATE_END){
        showTaskState();
        if(wall->isActive()){
            QTimer::singleShot(0, wall, SLOT(stop()));
        }
        stop();
    }
}

void TaskWallFollowing::showTaskState(){
    logger->info(taskState);
    emit newState(taskState);
    addData("taskState", taskState);
    emit dataChanged(this);
}


void TaskWallFollowing::controlFinishedWaypoints(QString waypoint){
    if(!isActive()){
        return;
    }

    if(waypoint == this->getSettingsValue("taskStartPoint").toString()){
        logger->info(this->getSettingsValue("taskStartPoint").toString() +" reached");
        QTimer::singleShot(0, navi, SLOT(clearGoal()));
        taskState = TASK_STATE_WALLFOLLOW_PART1;
        controlTaskStates();

    } else if(waypoint ==  this->getSettingsValue("wallAdjustPoint").toString()){
        logger->info(this->getSettingsValue("wallAdjustPoint").toString() +" reached");
        QTimer::singleShot(0, navi, SLOT(clearGoal()));
        taskState = TASK_STATE_WALLFOLLOW_PART2;
        controlTaskStates();

    } else {
        // None of current task waypoints reached oO
        logger->info("Something is wrong with navigation waypoints, abort...");
        emergencyStop();
    }
}



void TaskWallFollowing::controlAngleDistance(){
    if(!isActive()){
        return;
    }

    double alpha = 0.0;
    double dist = 0.0;

    if(this->getSettingsValue("useAdjustPoint").toBool()){

        if(taskState == TASK_STATE_WALLFOLLOW_PART1){
            dist = this->navi->getDistance(this->getSettingsValue("wallAdjustPoint").toString());
            if(dist < this->getSettingsValue("apDist").toDouble()){
                taskState = TASK_STATE_WALLFOLLOW_PART2;
                controlTaskStates();
            }
        } else if(taskState == TASK_STATE_WALLFOLLOW_PART2){
            alpha = this->navi->getAlpha(this->getSettingsValue("goal1point1").toString(), this->getSettingsValue("goal1point2").toString());
            dist = this->navi->getDistance(this->getSettingsValue("goal1point2").toString());
            if(alpha < 0 && dist < 3){
                taskState = TASK_STATE_END;
                controlTaskStates();
            }
        }

    } else {

        if(taskState == TASK_STATE_WALLFOLLOW_PART1){
            alpha = this->navi->getAlpha(this->getSettingsValue("goal1point1").toString(), this->getSettingsValue("goal1point2").toString());
            dist = this->navi->getDistance(this->getSettingsValue("goal1point2").toString());
            if(alpha < 0 && dist < 3){
                taskState = TASK_STATE_END;
                controlTaskStates();
            }
        }

    }
    addData("Alpha", alpha);
    addData("Dist", dist);
    emit dataChanged(this);

}


void TaskWallFollowing::stop(){
    if(!isActive()){
        logger->info("Not active!");
        return;
    }

    taskStopTimer.stop();
    taskState = TASK_STATE_END;
    showTaskState();

    calcTimer.stop();

    logger->info("Taskwallfollowing stopped");
    if(navi->hasGoal()){
        QTimer::singleShot(0, navi, SLOT(clearGoal()));
    }

    if(wall->isActive()){
        QTimer::singleShot(0, wall, SLOT(stop()));
    }

    active = false;
    setEnabled(false);
    emit finished(this,true);
}

void TaskWallFollowing::timeoutStop(){
    if(!isActive()){
        return;
    }

    taskStopTimer.stop();
    taskState = TASK_STATE_END_FAILED;
    showTaskState();

    calcTimer.stop();

    logger->info("Taskwallfollowing timeout stopped");
    if(navi->hasGoal()){
        QTimer::singleShot(0, navi, SLOT(clearGoal()));
    }

    if(wall->isActive()){
        QTimer::singleShot(0, wall, SLOT(stop()));
    }

    active = false;
    setEnabled(false);
    emit finished(this,false);
}


void TaskWallFollowing::emergencyStop(){
    if (!isActive()){
        logger->info("Not active, no emergency stop needed");
        return;
    }

    taskStopTimer.stop();
    taskState = TASK_STATE_END_FAILED;
    showTaskState();

    calcTimer.stop();

    logger->info("Taskwallfollowing emergency stopped");
    if(navi->hasGoal()){
        QTimer::singleShot(0, navi, SLOT(clearGoal()));
    }

    if(wall->isActive()){
        QTimer::singleShot(0, wall, SLOT(stop()));
    }

    active = false;
    setEnabled(false);
    emit finished(this,false);
}

void TaskWallFollowing::controlEnabledChanged(bool enabled){
    if(!enabled && isActive()){
        logger->info("Disable and deactivate TaskWallFollowing");
        stop();
    } else if(!enabled && !isActive()){
        //logger->info("Still deactivated");
    } else if(enabled && !isActive()){
        //logger->info("Enable and activate TaskWallFollowing");
        //startBehaviour();
    } else {
        //logger->info("Still activated");
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
    ret.append(navi);
    ret.append(sim);
    return ret;
}


QString TaskWallFollowing::getTaskState(){
    return taskState;
}
