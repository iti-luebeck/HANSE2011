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
    connect(this, SIGNAL(setRunDataSignal(int)), this, SLOT(setRunData(int)));

    setEnabled(false);
    running = false;
    counter = 1;

    // Default task settings
    this->setDefaultValue("taskStopTime",120000);
    this->setDefaultValue("signalTimer",1000);
    this->setDefaultValue("timerActivated", true);
    this->setDefaultValue("loopActivated", true);

    // Default pipefollow settings
    this->setDefaultValue("timer",250);
    this->setDefaultValue("threshold",200);
    this->setDefaultValue("debug",20);
    this->setDefaultValue("deltaAngle",176.0);
    this->setDefaultValue("deltaDist",5.0);
    this->setDefaultValue("kpDist",5.0);
    this->setDefaultValue("kpAngle",5.0);
    this->setDefaultValue("fwSpeed",0.3);
    this->setDefaultValue("robCenterX",320.0);
    this->setDefaultValue("robCenterY",176.0);
    this->setDefaultValue("maxDistance",320);
    this->setDefaultValue("badFrames",20);
    this->setDefaultValue("camWidth",640);
    this->setDefaultValue("camHeight",480);
    this->setDefaultValue("enableUIOutput", true);
    this->setDefaultValue("convColor", 2);

    // Default navigation settings
    this->setDefaultValue("taskStartPoint", "ps");
    this->setDefaultValue("goal1point1", "go1p1");
    this->setDefaultValue("goal1point2", "go1p2");
    this->setDefaultValue("goal2point1", "go2p1");
    this->setDefaultValue("goal2point2", "go2p2");
    this->setDefaultValue("goal3point1", "go3p1");
    this->setDefaultValue("goal3point2", "go3p2");
    this->setDefaultValue("gate1point", "ga1");
    this->setDefaultValue("gate2point", "ga2");


    this->setDefaultValue("targetTolerance", 10);

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
        if(this->getSettingsValue("useStartNavigation").toBool() == true){
            emit newStateOverview("Move to start");
        }
        emit newStateOverview(TASK_STATE_MOVE_TO_TASK_START);
        emit newStateOverview(TASK_STATE_MOVE_TO_PIPE_INIT);
        emit newStateOverview(TASK_STATE_PIPEFOLLOW_PART1);
        emit newStateOverview(TASK_STATE_MOVE_TO_GATEWAYPOINT1);
        emit newStateOverview(TASK_STATE_MOVE_TO_PIPE);
        emit newStateOverview(TASK_STATE_PIPEFOLLOW_PART2);
        emit newStateOverview(TASK_STATE_MOVE_TO_GATEWAYPOINT2);
        if(this->getSettingsValue("loopActivated").toBool()){
            emit newStateOverview("...in a loop");
        }
    }

    setEnabled(true);
    emit started(this);
    running = true;

    angleTimer.start(100);

    // Enable all components
    if(!this->navi->isEnabled()){
        this->navi->setEnabled(true);
    }

    addData("loop", this->getSettingsValue("loopActivated").toBool());
    addData("stoptimer", this->getSettingsValue("timerActivated").toBool());
    emit dataChanged(this);


    emit updateSettings();

    if(this->getSettingsValue("timerActivated").toBool()){
        logger->debug("TaskPipeFollowing with timer stop");
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
    if(this->isEnabled()){
        if(taskState != TASK_STATE_MOVE_TO_PIPE_INIT){
            taskState = TASK_STATE_MOVE_TO_PIPE_INIT;
            logger->info(taskState);
            addData("taskState", taskState);
            emit newState(taskState);
            emit dataChanged(this);

            emit setUpdatePixmapSignal(this->getSettingsValue("enableUIOutput").toBool());
            this->pipe->setSettingsValue("timer",this->getSettingsValue("timer"));
            this->pipe->setSettingsValue("threshold",this->getSettingsValue("threshold"));
            this->pipe->setSettingsValue("debug",this->getSettingsValue("debug"));
            this->pipe->setSettingsValue("deltaAngle",this->getSettingsValue("deltaAngle"));
            this->pipe->setSettingsValue("deltaDist",this->getSettingsValue("deltaDist"));
            this->pipe->setSettingsValue("kpDist",this->getSettingsValue("kpDist"));
            this->pipe->setSettingsValue("kpAngle",this->getSettingsValue("kpAngle"));
            this->pipe->setSettingsValue("fwSpeed",this->getSettingsValue("fwSpeed"));
            this->pipe->setSettingsValue("robCenterX",this->getSettingsValue("robCenterX"));
            this->pipe->setSettingsValue("robCenterY",this->getSettingsValue("robCenterY"));
            this->pipe->setSettingsValue("maxDistance",this->getSettingsValue("maxDistance"));
            this->pipe->setSettingsValue("badFrames",this->getSettingsValue("badFrames"));

            if(!this->pipe->isEnabled()){
                logger->debug("enable pipefollow");
                QTimer::singleShot(0, pipe, SLOT(startBehaviour()));
            } else {
                QTimer::singleShot(0, pipe, SLOT(reset()));
            }
            QTimer::singleShot(100, this, SLOT(moveToPipeStart()));
        } else {
            if(flag_PF_Pipe_Seen_Start){
                // State finished, change state
                QTimer::singleShot(0, this, SLOT(pipefollowPart1()));
            } else {
                // State not finished, stay at this state
                QTimer::singleShot(100, this, SLOT(moveToPipeStart()));
            }
        }
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

            emit setUpdatePixmapSignal(this->getSettingsValue("enableUIOutput").toBool());
            this->pipe->setSettingsValue("timer",this->getSettingsValue("timer"));
            this->pipe->setSettingsValue("threshold",this->getSettingsValue("threshold"));
            this->pipe->setSettingsValue("debug",this->getSettingsValue("debug"));
            this->pipe->setSettingsValue("deltaAngle",this->getSettingsValue("deltaAngle"));
            this->pipe->setSettingsValue("deltaDist",this->getSettingsValue("deltaDist"));
            this->pipe->setSettingsValue("kpDist",this->getSettingsValue("kpDist"));
            this->pipe->setSettingsValue("kpAngle",this->getSettingsValue("kpAngle"));
            this->pipe->setSettingsValue("fwSpeed",this->getSettingsValue("fwSpeed"));
            this->pipe->setSettingsValue("robCenterX",this->getSettingsValue("robCenterX"));
            this->pipe->setSettingsValue("robCenterY",this->getSettingsValue("robCenterY"));
            this->pipe->setSettingsValue("maxDistance",this->getSettingsValue("maxDistance"));
            this->pipe->setSettingsValue("badFrames",this->getSettingsValue("badFrames"));

            if(!this->pipe->isEnabled()){
                logger->debug("enable pipefollow");
                QTimer::singleShot(0, pipe, SLOT(startBehaviour()));
            } else {
                QTimer::singleShot(0, pipe, SLOT(reset()));
            }
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
        logger->info("flag_PF_Pipe_Seen = true");
    } else if(newState == PIPE_STATE_PASSED && currentState == TASK_STATE_PIPEFOLLOW_PART1){
        flag_PF_Part_1_Finished = true;
        logger->info("flag_PF_Part_1_Finished = true");
    } else if(newState == PIPE_STATE_IS_SEEN && currentState == TASK_STATE_MOVE_TO_PIPE){
        flag_PF_Pipe_Seen = true;
        logger->info("flag_PF_Pipe_Seen = true");
    } else if(newState == PIPE_STATE_PASSED && currentState == TASK_STATE_PIPEFOLLOW_PART2){
        flag_PF_Part_2_Finished = true;
        logger->info("flag_PF_Part_2_Finished = true");
    }
}

void TaskPipeFollowing::controlFinishedWaypoints(QString waypoint){
    if(this->isEnabled()){
        if(waypoint == this->getSettingsValue("taskStartPoint").toString()){
            // Task startwaypoint reached, move to pipe
            QTimer::singleShot(0, navi, SLOT(clearGoal()));
            QTimer::singleShot(0, this, SLOT(moveToPipeStart()));
        } else if(waypoint ==  this->getSettingsValue("gate1point").toString()){
            // Gate 1 waypoint reached, move back to pipe
            QTimer::singleShot(0, navi, SLOT(clearGoal()));
            QTimer::singleShot(0, this, SLOT(moveToPipe()));
        } else if(waypoint ==  this->getSettingsValue("gate2point").toString()){
            // Gate 2 waypoint reached, task finished
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
        if(currentState == TASK_STATE_PIPEFOLLOW_PART1){
            alpha = this->navi->getAlpha(this->getSettingsValue("goal1point1").toString(), this->getSettingsValue("goal1point2").toString());
            if(fabs(alpha) < 10){
                flag_GoalLine_1_reached = true;
                logger->info("flag_GoalLine_1_reached = true");
            }
        } else if(currentState == TASK_STATE_PIPEFOLLOW_PART2){
            alpha = this->navi->getAlpha(this->getSettingsValue("goal2point1").toString(), this->getSettingsValue("goal2point2").toString());
            if(fabs(alpha) < 10){
                flag_GoalLine_2_reached = true;
                logger->info("flag_GoalLine_2_reached = true");
            }
        }
        addData("Alpha", alpha);
        emit dataChanged(this);
    }
}

void TaskPipeFollowing::stop(){
    taskState = TASK_STATE_END;
    logger->info(taskState);
    addData("taskState", taskState);
    emit newState(taskState);
    emit dataChanged(this);
    if(this->isEnabled()){
        running = false;
        logger->info("Taskpipefollowing stopped");

        if (this->isActive())
        {
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
