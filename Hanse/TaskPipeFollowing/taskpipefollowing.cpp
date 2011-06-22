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

    connect(this,SIGNAL(setUpdatePixmapSignal(bool)),pipe,SLOT(setUpdatePixmapSlot(bool)));

    setEnabled(false);
    running = false;
    counter = 1;

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

    angleTimer.moveToThread(this);

    connect(this, SIGNAL(enabled(bool)), this, SLOT(controlEnabledChanged(bool)));
    connect(navi, SIGNAL(reachedWaypoint(QString)), this, SLOT(controlFinishedWaypoints(QString)));
    connect(turn180, SIGNAL(turn180finished(QString)), this, SLOT(controlFinishedWaypoints(QString)));
    connect(pipe, SIGNAL(newPipeState(QString)), this, SLOT(controlPipeState(QString)));
    connect(&angleTimer,SIGNAL(timeout()),this,SLOT(controlAngleCalculation()));
}

bool TaskPipeFollowing::isActive(){
    return isEnabled();
}


void TaskPipeFollowing::init(){
    logger->debug("taskpipefollowing init");
}

void TaskPipeFollowing::reset() {
    RobotModule::reset();

    flag_PF_Pipe_Seen_Start = false;
    flag_PF_Part_1_Finished = false;
    flag_GoalLine_1_reached = false;
    flag_PF_Pipe_Seen = false;
    flag_PF_Part_2_Finished = false;
    flag_GoalLine_2_reached = false;
    taskState = TASK_STATE_START;

}


void TaskPipeFollowing::startBehaviour(){
    if (this->isEnabled()){
        logger->info("Already enabled/started!");
        return;
    }
    this->reset();

    running = true;
    setHealthToOk();

    taskState = TASK_STATE_START;
    logger->info(taskState);
    addData("taskState", taskState);
    emit newState(taskState);
    emit dataChanged(this);

    if(!this->isEnabled()){
        emit newStateOverview(TASK_STATE_MOVE_TO_TASK_START);
        emit newStateOverview(TASK_STATE_MOVE_TO_PIPE_INIT);
        emit newStateOverview(TASK_STATE_PIPEFOLLOW_PART1);
        emit newStateOverview(TASK_STATE_MOVE_TO_GATEWAYPOINT1);
        emit newStateOverview(TASK_STATE_MOVE_TO_PIPE);
        emit newStateOverview(TASK_STATE_PIPEFOLLOW_PART2);
        emit newStateOverview(TASK_STATE_MOVE_TO_GATEWAYPOINT2);
    }

    setEnabled(true);
    emit started(this);
    running = true;

    angleTimer.start(100);

    // Enable all components
    if(!this->navi->isEnabled()){
        this->navi->setEnabled(true);
    }

    addData("stoptimer", this->getSettingsValue("timerActivated").toBool());
    emit dataChanged(this);


    emit updateSettings();

    if(this->getSettingsValue("timerActivated").toBool()){
        logger->info("TaskPipeFollowing with timer stop");
        taskTimer.singleShot(this->getSettingsValue("taskStopTime").toInt(),this, SLOT(timeoutStop()));
        taskTimer.start();
    }

    QTimer::singleShot(0, this, SLOT(moveToTaskStart()));
}


void TaskPipeFollowing::moveToTaskStart(){
    if(this->isEnabled() && this->navi->getHealthStatus().isHealthOk()){
        taskState = TASK_STATE_MOVE_TO_TASK_START;
        logger->info(taskState);
        addData("taskState", taskState);
        emit newState(taskState);
        emit dataChanged(this);
        this->navi->gotoWayPoint(this->getSettingsValue("taskStartPoint").toString());
    } else {
        logger->info("Something is wrong with navigation/task, abort...");
        QTimer::singleShot(0, this, SLOT(emergencyStop()));
    }
}

void TaskPipeFollowing::moveToPipeStart(){
    if(this->isEnabled() && this->navi->getHealthStatus().isHealthOk()){
        taskState = TASK_STATE_MOVE_TO_PIPE_INIT;
        logger->info(taskState);
        addData("taskState", taskState);
        emit newState(taskState);
        emit dataChanged(this);
        this->navi->gotoWayPoint(this->getSettingsValue("pipeStartPoint").toString());
    } else {
        logger->info("Something is wrong with navigation/task, abort...");
        QTimer::singleShot(0, this, SLOT(emergencyStop()));
    }
}


void TaskPipeFollowing::pipefollowPart1(){
    if(this->isEnabled()){
        if(taskState != TASK_STATE_PIPEFOLLOW_PART1){
            taskState = TASK_STATE_PIPEFOLLOW_PART1;
            logger->info(taskState);
            addData("taskState", taskState);
            emit newState(taskState);
            emit dataChanged(this);


            logger->info("Enable pipefollow");
            QTimer::singleShot(0, pipe, SLOT(startBehaviour()));
        }

        if(flag_PF_Part_1_Finished || flag_GoalLine_1_reached){
            // End of pipe reached
            QTimer::singleShot(0, pipe, SLOT(stop()));
            QTimer::singleShot(0, this, SLOT(moveToGatewaypoint1()));
        } else {
            // State not finished, stay at this state
            QTimer::singleShot(100, this, SLOT(pipefollowPart1()));
        }
    }
}

void TaskPipeFollowing::moveToGatewaypoint1(){
    if(this->isEnabled()){
        taskState = TASK_STATE_MOVE_TO_GATEWAYPOINT1;
        logger->info(taskState);
        addData("taskState", taskState);
        emit newState(taskState);
        this->navi->gotoWayPoint(this->getSettingsValue("gate1point").toString());
    }
}

void TaskPipeFollowing::moveToPipe(){
    if(this->isEnabled()){
        if(taskState != TASK_STATE_MOVE_TO_PIPE){
            taskState = TASK_STATE_MOVE_TO_PIPE;
            logger->info(taskState);
            addData("taskState", taskState);
            emit newState(taskState);
            emit dataChanged(this);

            logger->info("Enable pipefollow");
            QTimer::singleShot(0, pipe, SLOT(startBehaviour()));

            QTimer::singleShot(100, this, SLOT(moveToPipe()));
        } else {
            if(flag_PF_Pipe_Seen){
                // State finished, change state
                QTimer::singleShot(0, this, SLOT(pipefollowPart2()));
            } else {
                // State not finished, stay at this state
                QTimer::singleShot(100, this, SLOT(moveToPipe()));
            }
        }
    }
}

void TaskPipeFollowing::pipefollowPart2(){
    if(this->isEnabled()){
        if(taskState != TASK_STATE_PIPEFOLLOW_PART2){
            taskState = TASK_STATE_PIPEFOLLOW_PART2;
            logger->info(taskState);
            addData("taskState", taskState);
            emit newState(taskState);
            emit dataChanged(this);
        }

        if(flag_PF_Part_2_Finished || flag_GoalLine_2_reached ){
            // End of pipe reached
            QTimer::singleShot(0, pipe, SLOT(stop()));
            QTimer::singleShot(0, this, SLOT(moveToGatewaypoint2()));
        } else {
            // State not finished, stay at this state
            QTimer::singleShot(100, this, SLOT(pipefollowPart2()));
        }


    }
}

void TaskPipeFollowing::moveToGatewaypoint2(){
    if(this->isEnabled()){
        taskState = TASK_STATE_MOVE_TO_GATEWAYPOINT2;
        logger->info(taskState);
        addData("taskState", taskState);
        emit newState(taskState);
        this->navi->gotoWayPoint(this->getSettingsValue("gate2point").toString());
    }
}

void TaskPipeFollowing::controlPipeState(QString newState){
    QString currentState = this->getTaskState();
    if(newState == PIPE_STATE_IS_SEEN && currentState == TASK_STATE_MOVE_TO_PIPE_INIT){
        flag_PF_Pipe_Seen_Start = true;
        logger->info("flag_PF_Pipe_Seen = true... through PIPE_STATE");
    } else if(newState == PIPE_STATE_PASSED && currentState == TASK_STATE_PIPEFOLLOW_PART1){
        flag_PF_Part_1_Finished = true;
        logger->info("flag_PF_Part_1_Finished = true... through PIPE_STATE");
    } else if(newState == PIPE_STATE_IS_SEEN && currentState == TASK_STATE_MOVE_TO_PIPE){
        flag_PF_Pipe_Seen = true;
        logger->info("flag_PF_Pipe_Seen = true... through PIPE_STATE");
    } else if(newState == PIPE_STATE_PASSED && currentState == TASK_STATE_PIPEFOLLOW_PART2){
        flag_PF_Part_2_Finished = true;
        logger->info("flag_PF_Part_2_Finished = true... through PIPE_STATE");
    }
}

void TaskPipeFollowing::controlFinishedWaypoints(QString waypoint){
    if(this->isEnabled()){
        if(waypoint == this->getSettingsValue("taskStartPoint").toString()){
            // Task startwaypoint reached, move to pipe
            logger->info(this->getSettingsValue("taskStartPoint").toString() +" reached");
            QTimer::singleShot(0, navi, SLOT(clearGoal()));
            QTimer::singleShot(0, this, SLOT(moveToPipeStart()));
        } else if(waypoint ==  this->getSettingsValue("pipeStartPoint").toString()){
            // Pipe startwaypoint reached, start pipefollow
            logger->info(this->getSettingsValue("pipeStartPoint").toString() +" reached");
            QTimer::singleShot(0, navi, SLOT(clearGoal()));
            QTimer::singleShot(0, this, SLOT(pipefollowPart1()));
        }else if(waypoint ==  this->getSettingsValue("gate1point").toString()){
            // Gate 1 waypoint reached, move back to pipe
            logger->info(this->getSettingsValue("gate1point").toString() +" reached");
            QTimer::singleShot(0, navi, SLOT(clearGoal()));
            QTimer::singleShot(0, this, SLOT(moveToPipe()));
        } else if(waypoint ==  this->getSettingsValue("gate2point").toString()){
            // Gate 2 waypoint reached, task finished
            logger->info(this->getSettingsValue("gate2point").toString() +" reached");
            QTimer::singleShot(0, navi, SLOT(clearGoal()));
            QTimer::singleShot(0, this, SLOT(stop()));
        } else {
            // Error!
            logger->info("Something is wrong with navigation waypoints, abort...");
            QTimer::singleShot(0, this, SLOT(emergencyStop()));
        }
    }
}

void TaskPipeFollowing::controlAngleCalculation(){
    if(this->isEnabled()){
        QString currentState = this->getTaskState();
        double alpha = 0.0;
        double dist = 0.0;
        if(currentState == TASK_STATE_PIPEFOLLOW_PART1){
            alpha = this->navi->getAlpha(this->getSettingsValue("goal1point1").toString(), this->getSettingsValue("goal1point2").toString());
            dist = this->navi->getDistance(this->getSettingsValue("goal1point2").toString());
            if(fabs(alpha) <  this->getSettingsValue("angle").toDouble() && dist < 2){
                flag_GoalLine_1_reached = true;
                logger->info("flag_GoalLine_1_reached = true... through angle");
            }
        } else if(currentState == TASK_STATE_PIPEFOLLOW_PART2){
            alpha = this->navi->getAlpha(this->getSettingsValue("goal2point1").toString(), this->getSettingsValue("goal2point2").toString());
            dist = this->navi->getDistance(this->getSettingsValue("goal2point2").toString());
            if(fabs(alpha) < this->getSettingsValue("angle").toDouble() && dist < 2){
                flag_GoalLine_2_reached = true;
                logger->info("flag_GoalLine_2_reached = true... through angle");
            }
        }
        addData("Alpha", alpha);
        addData("Dist", dist);
        emit dataChanged(this);
    }
}

void TaskPipeFollowing::stop(){
    if(this->isEnabled()){
        taskState = TASK_STATE_END;
        logger->info(taskState);
        addData("taskState", taskState);
        emit newState(taskState);
        emit dataChanged(this);

        running = false;
        logger->info("Taskpipefollowing stopped");

        if (this->isActive())
        {
            QTimer::singleShot(0, navi, SLOT(clearGoal()));
            this->taskTimer.stop();
            QTimer::singleShot(0, pipe, SLOT(stop()));
            this->setEnabled(false);
            emit finished(this,true);
        }
    } else {
        logger->info("Something is really wrong...");
        emit finished(this,false);
    }
}

void TaskPipeFollowing::timeoutStop(){
    if(this->isEnabled()){
        taskState = TASK_STATE_END_FAILED;
        logger->info(taskState);
        addData("taskState", taskState);
        emit newState(taskState);
        emit dataChanged(this);

        running = false;
        logger->info("Taskpipefollowing timeout stopped");

        if (this->isActive())
        {
            QTimer::singleShot(0, navi, SLOT(clearGoal()));
            this->taskTimer.stop();
            QTimer::singleShot(0, pipe, SLOT(stop()));
            this->setEnabled(false);
            emit finished(this,false);
        }
    }
}


void TaskPipeFollowing::emergencyStop(){
    if (!this->isEnabled()){
        logger->info("Emergency stop: Not enabled!");
        return;
    }

    taskState = TASK_STATE_END_FAILED;
    logger->info(taskState);
    addData("taskState", taskState);
    emit newState(taskState);
    emit dataChanged(this);

    running = false;
    logger->info( "Task pipefollowing emergency stopped" );

    if (this->isActive())
    {
        QTimer::singleShot(0, navi, SLOT(clearGoal()));
        this->taskTimer.stop();
        QTimer::singleShot(0, pipe, SLOT(stop()));
        this->setEnabled(false);
        emit finished(this,false);
    }
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

void TaskPipeFollowing::controlEnabledChanged(bool b){
    if(b == false){
        logger->info("No longer enabled!");
        QTimer::singleShot(0, this, SLOT(emergencyStop()));
    }
}

QString TaskPipeFollowing::getTaskState(){
    return taskState;
}
