#include "taskpipefollowing.h"
#include "taskpipefollowingform.h"
#include <QtGui>
#include <Module_Simulation/module_simulation.h>
#include <Module_Navigation/module_navigation.h>
#include <Behaviour_PipeFollowing/behaviour_pipefollowing.h>
#include <Behaviour_TurnOneEighty/behaviour_turnoneeighty.h>

TaskPipeFollowing::TaskPipeFollowing(QString id, Behaviour_PipeFollowing *w, Module_Simulation *sim, Module_Navigation *n, Behaviour_TurnOneEighty *o180)
    : RobotBehaviour(id)
{
    this->sim = sim;
    this->pipe = w;
    this->turn180 = o180;
    this->navi = n;
}

bool TaskPipeFollowing::isActive(){
    return active;
}


void TaskPipeFollowing::init(){
    logger->debug("taskpipefollowing init");

    active = false;
    setEnabled(false);

    connect(this,SIGNAL(setUpdatePixmapSignal(bool)),pipe,SLOT(setUpdatePixmapSlot(bool)));

    // Default task settings
    this->setDefaultValue("taskStopTime",120000);
    this->setDefaultValue("timerActivated", true);

    // Default navigation settings
    this->setDefaultValue("taskStartPoint", "tSP");
    this->setDefaultValue("pipeStartPoint", "pSP");
    this->setDefaultValue("goal1point1", "go1p1");
    this->setDefaultValue("goal1point2", "go1p2");
    this->setDefaultValue("goal2point1", "go2p1");
    this->setDefaultValue("goal2point2", "go2p2");
    this->setDefaultValue("goal3point1", "go3p1");
    this->setDefaultValue("goal3point2", "go3p2");
    this->setDefaultValue("gate1point", "ga1p");
    this->setDefaultValue("gate2point", "ga2p");
    this->setDefaultValue("angle", 15);

    // Default turn180 settings
    this->setDefaultValue("hysteresis", 10);
    this->setDefaultValue("p", 0.4);
    this->setDefaultValue("degree", 180);

    taskTimer.setSingleShot(true);
    taskTimer.moveToThread(this);
    calcTimer.moveToThread(this);

    connect(this, SIGNAL(enabled(bool)), this, SLOT(controlEnabledChanged(bool)));
    connect(navi, SIGNAL(reachedWaypoint(QString)), this, SLOT(controlFinishedWaypoints(QString)));
    connect(turn180, SIGNAL(turn180finished(QString)), this, SLOT(controlFinishedWaypoints(QString)));
    connect(pipe, SIGNAL(newPipeState(QString)), this, SLOT(controlPipeState(QString)));
    connect(&calcTimer,SIGNAL(timeout()),this,SLOT(controlAngleDistance()));
}

void TaskPipeFollowing::reset() {
    RobotModule::reset();

    taskState = TASK_STATE_START;
}


void TaskPipeFollowing::startBehaviour(){
    if (isActive()){
        logger->info("Already active!");
        return;
    }

    this->reset();


    setHealthToOk();

    taskState = TASK_STATE_START;
    showTaskState();

    active = true;
    if(!isEnabled()){
        setEnabled(true);
    }
    emit started(this);

    emit newStateOverview(TASK_STATE_MOVE_TO_PIPE_INIT);
    emit newStateOverview(TASK_STATE_PIPEFOLLOW_PART1);
    emit newStateOverview(TASK_STATE_MOVE_TO_GATEWAYPOINT1);
    emit newStateOverview(TASK_STATE_MOVE_TO_PIPE);
    emit newStateOverview(TASK_STATE_PIPEFOLLOW_PART2);
    emit newStateOverview(TASK_STATE_MOVE_TO_GATEWAYPOINT2);

    calcTimer.start(100);

    if(!this->navi->isEnabled()){
        this->navi->setEnabled(true);
    }

    emit updateSettings();

    if(this->getSettingsValue("timerActivated").toBool()){
        logger->info("TaskPipeFollowing with timer stop");
        taskTimer.singleShot(this->getSettingsValue("taskStopTime").toInt(),this, SLOT(timeoutStop()));
        taskTimer.start();
    }

    taskState = TASK_STATE_MOVE_TO_TASK_START;
}

void TaskPipeFollowing::controlTaskStates(){
    if(!isActive()){
        return;
    }

    if(taskState == TASK_STATE_MOVE_TO_TASK_START){
        showTaskState();
        this->navi->gotoWayPoint(this->getSettingsValue("taskStartPoint").toString());

    } else if(taskState == TASK_STATE_MOVE_TO_PIPE_INIT){
        showTaskState();
        this->navi->gotoWayPoint(this->getSettingsValue("pipeStartPoint").toString());

    } else if(taskState == TASK_STATE_PIPEFOLLOW_PART1){
        showTaskState();
        if(this->pipe->getHealthStatus().isHealthOk()){
            if (!this->pipe->isActive()) {
                logger->info("Activate pipefollowing");
                QTimer::singleShot(0, pipe, SLOT(startBehaviour()));
            } else {
                QTimer::singleShot(0, pipe, SLOT(reset()));
            }
        } else {
            logger->info("Pipefollowing not possible");
            taskState = TASK_STATE_MOVE_TO_GATEWAYPOINT1;
            controlTaskStates();
        }

    } else if(taskState == TASK_STATE_MOVE_TO_GATEWAYPOINT1){
        showTaskState();
        if(pipe->isActive()){
            QTimer::singleShot(0, pipe, SLOT(stop()));
        }
        this->navi->gotoWayPoint(this->getSettingsValue("gate1point").toString());


    } else if(taskState == TASK_STATE_MOVE_TO_PIPE){
        showTaskState();
        if(this->pipe->getHealthStatus().isHealthOk()){
            if (!this->pipe->isActive()) {
                logger->info("Activate pipefollowing");
                QTimer::singleShot(0, pipe, SLOT(startBehaviour()));
            } else {
                QTimer::singleShot(0, pipe, SLOT(reset()));
            }
        } else {
            logger->info("Pipefollowing not possible");
            taskState = TASK_STATE_MOVE_TO_GATEWAYPOINT2;
            controlTaskStates();
        }

    } else if(taskState == TASK_STATE_MOVE_TO_GATEWAYPOINT2){
        showTaskState();
        if(pipe->isActive()){
            QTimer::singleShot(0, pipe, SLOT(stop()));
        }
        this->navi->gotoWayPoint(this->getSettingsValue("gate2point").toString());

    } else if(taskState == TASK_STATE_END){
        showTaskState();
        if(pipe->isActive()){
            QTimer::singleShot(0, pipe, SLOT(stop()));
        }
        stop();
    }
}

void TaskPipeFollowing::controlFinishedWaypoints(QString waypoint){
    if(!isActive()){
        return;
    }

    if(waypoint == this->getSettingsValue("taskStartPoint").toString()){
        logger->info(this->getSettingsValue("taskStartPoint").toString() +" reached");
        QTimer::singleShot(0, navi, SLOT(clearGoal()));
        taskState = TASK_STATE_MOVE_TO_PIPE_INIT;
        controlTaskStates();

    } else if(waypoint ==  this->getSettingsValue("pipeStartPoint").toString()){
        logger->info(this->getSettingsValue("pipeStartPoint").toString() +" reached");
        QTimer::singleShot(0, navi, SLOT(clearGoal()));
        taskState = TASK_STATE_PIPEFOLLOW_PART1;
        controlTaskStates();

    }else if(waypoint ==  this->getSettingsValue("gate1point").toString()){
        logger->info(this->getSettingsValue("gate1point").toString() +" reached");
        QTimer::singleShot(0, navi, SLOT(clearGoal()));
        taskState = TASK_STATE_MOVE_TO_PIPE;
        controlTaskStates();

    } else if(waypoint ==  this->getSettingsValue("gate2point").toString()){
        logger->info(this->getSettingsValue("gate2point").toString() +" reached");
        QTimer::singleShot(0, navi, SLOT(clearGoal()));
        taskState = TASK_STATE_END;
        controlTaskStates();

    } else {
        // None of current task waypoints reached oO
        logger->info("Something is wrong with navigation waypoints, abort...");
        emergencyStop();;
    }

}

void TaskPipeFollowing::controlPipeState(QString newState){
    if(!isActive()){
        return;
    }

    if(newState == STATE_PASSED && taskState == TASK_STATE_PIPEFOLLOW_PART1){
        this->dataLockerMutex.lock();
        taskState = TASK_STATE_MOVE_TO_GATEWAYPOINT1;
        this->dataLockerMutex.unlock();
        controlTaskStates();

    } else if(newState == STATE_PASSED && taskState == TASK_STATE_PIPEFOLLOW_PART2){
        this->dataLockerMutex.lock();
        taskState = TASK_STATE_MOVE_TO_GATEWAYPOINT2;
        this->dataLockerMutex.unlock();
        controlTaskStates();
    }
}



void TaskPipeFollowing::controlAngleDistance(){
    if(!isActive()){
        return;
    }
    double alpha = 0.0;
    double dist = 0.0;
    if(taskState == TASK_STATE_PIPEFOLLOW_PART1){
        alpha = this->navi->getAlpha(this->getSettingsValue("goal1point1").toString(), this->getSettingsValue("goal1point2").toString());
        dist = this->navi->getDistance(this->getSettingsValue("goal1point2").toString());
        if(alpha <  0 && dist < 3){
            this->dataLockerMutex.lock();
            taskState = TASK_STATE_MOVE_TO_GATEWAYPOINT1;
            this->dataLockerMutex.unlock();
            controlTaskStates();
        }
    } else if(taskState == TASK_STATE_PIPEFOLLOW_PART2){
        alpha = this->navi->getAlpha(this->getSettingsValue("goal2point1").toString(), this->getSettingsValue("goal2point2").toString());
        dist = this->navi->getDistance(this->getSettingsValue("goal2point2").toString());
        if(alpha <  0 && dist < 3){
            this->dataLockerMutex.lock();
            taskState = TASK_STATE_MOVE_TO_GATEWAYPOINT2;
            this->dataLockerMutex.unlock();
            controlTaskStates();
        }
    }
    addData("Angle", alpha);
    addData("Dist", dist);
    emit dataChanged(this);
}

void TaskPipeFollowing::initBehaviourParameters(){
    // Maybe TODO
}

void TaskPipeFollowing::showTaskState(){
    logger->info(taskState);
    emit newState(taskState);
    addData("taskState", taskState);
    emit dataChanged(this);
}

void TaskPipeFollowing::stop(){
    if(!isActive()){
        logger->info("Not active!");
        return;
    }

    logger->info("Taskpipefollowing stopped");

    active = false;
    setEnabled(false);
    calcTimer.stop();
    taskState = TASK_STATE_END;
    showTaskState();

    if(navi->hasGoal()){
        QTimer::singleShot(0, navi, SLOT(clearGoal()));
    }

    if(pipe->isActive()){
        QTimer::singleShot(0, pipe, SLOT(stop()));
    }

    this->taskTimer.stop();
    emit finished(this,true);
}

void TaskPipeFollowing::timeoutStop(){
    if(!isActive()){
        logger->info("Not active!");
        return;
    }

    logger->info("Taskpipefollowing timeout stopped");

    active = false;
    setEnabled(false);
    calcTimer.stop();
    taskState = TASK_STATE_END_FAILED;
    showTaskState();

    if(navi->hasGoal()){
        QTimer::singleShot(0, navi, SLOT(clearGoal()));
    }

    if(pipe->isActive()){
        QTimer::singleShot(0, pipe, SLOT(stop()));
    }

    this->taskTimer.stop();
    emit finished(this,false);
}


void TaskPipeFollowing::emergencyStop(){
    if(!isActive()){
        logger->info("Not active, no emergency stop needed");
        return;
    }

    logger->info("Taskpipefollowing emergency stopped");

    active = false;
    setEnabled(false);
    calcTimer.stop();
    taskState = TASK_STATE_END_FAILED;
    showTaskState();

    if(navi->hasGoal()){
        QTimer::singleShot(0, navi, SLOT(clearGoal()));
    }

    if(pipe->isActive()){
        QTimer::singleShot(0, pipe, SLOT(stop()));
    }

    this->taskTimer.stop();
    emit finished(this,false);
}


void TaskPipeFollowing::terminate()
{
    this->stop();
    RobotModule::terminate();
}

QWidget* TaskPipeFollowing::createView(QWidget *parent)
{
    return new TaskPipeFollowingForm(this, parent);
}

QList<RobotModule*> TaskPipeFollowing::getDependencies()
{
    QList<RobotModule*> ret;
    ret.append(pipe);
    ret.append(navi);
    ret.append(sim);
    return ret;
}

void TaskPipeFollowing::controlEnabledChanged(bool enabled){
    if(!enabled && isActive()){
        logger->info("Disable and deactivate TaskPipeFollowing");
        stop();
    } else if(!enabled && !isActive()){
        //logger->info("Still deactivated");
    } else if(enabled && !isActive()){
        //logger->info("Enable and activate TaskPipeFollowing");
        //startBehaviour();
    } else {
        //logger->info("Still activated");
    }
}

QString TaskPipeFollowing::getTaskState(){
    return taskState;
}
