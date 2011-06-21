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

    setEnabled(false);
    running = false;
    counter = 1;

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
    connect(&angleTimer,SIGNAL(timeout()),this,SLOT(controlAngleCalculation()));
}

bool TaskWallFollowing::isActive(){
    return isEnabled();
}


void TaskWallFollowing::init(){
    logger->debug("taskwallfollowing init");
}

void TaskWallFollowing::reset() {
    RobotModule::reset();

}


void TaskWallFollowing::startBehaviour(){
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
        emit newStateOverview(TASK_STATE_WALLFOLLOW_PART1);
        emit newStateOverview(TASK_STATE_ADJUST_HEADING);
        emit newStateOverview(TASK_STATE_WALLFOLLOW_PART2);
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
        logger->info("TaskWallFollowing with timer stop");
        taskTimer.singleShot(this->getSettingsValue("taskStopTime").toInt(),this, SLOT(timeoutStop()));
        taskTimer.start();
    }

    QTimer::singleShot(0, this, SLOT(moveToTaskStart()));
}


void TaskWallFollowing::moveToTaskStart(){
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


void TaskWallFollowing::wallfollowPart1(){
    if(this->isEnabled()){
        if(taskState != TASK_STATE_WALLFOLLOW_PART1){
            taskState = TASK_STATE_WALLFOLLOW_PART1;
            logger->info(taskState);
            addData("taskState", taskState);
            emit newState(taskState);
            emit dataChanged(this);

            logger->info("Enable wallfollow");
            QTimer::singleShot(0, wall, SLOT(startBehaviour()));

            QTimer::singleShot(100, this, SLOT(wallfollowPart1()));
        } else {
            if(this->getSettingsValue("useAdjustPoint").toBool()){
                if(flag_AdjustHeading){
                    // State finished, change state
                    QTimer::singleShot(0, wall, SLOT(stop()));
                    QTimer::singleShot(0, this, SLOT(adjustHeading()));
                } else {
                    // State not finished, stay at this state
                    QTimer::singleShot(100, this, SLOT(wallfollowPart1()));
                }
            } else {
                if(flag_GoalLine_reached){
                    QTimer::singleShot(0, wall, SLOT(stop()));
                    QTimer::singleShot(0, this, SLOT(stop()));
                } else {
                    QTimer::singleShot(100, this, SLOT(wallfollowPart1()));
                }
            }
        }
    }
}

void TaskWallFollowing::adjustHeading(){
    if(this->isEnabled()){
        taskState = TASK_STATE_ADJUST_HEADING;
        logger->info(taskState);
        addData("taskState", taskState);
        emit newState(taskState);
        this->navi->gotoWayPoint(this->getSettingsValue("wallAdjustPoint").toString());
    }
}

void TaskWallFollowing::wallfollowPart2(){
    if(this->isEnabled()){
        if(taskState != TASK_STATE_WALLFOLLOW_PART2){
            taskState = TASK_STATE_WALLFOLLOW_PART2;
            logger->info(taskState);
            addData("taskState", taskState);
            emit newState(taskState);
            emit dataChanged(this);

            logger->info("Enable wallfollow");
            QTimer::singleShot(0, wall, SLOT(startBehaviour()));

            QTimer::singleShot(100, this, SLOT(wallfollowPart2()));
        } else {
            if(flag_GoalLine_reached){
                QTimer::singleShot(0, wall, SLOT(stop()));
                QTimer::singleShot(0, this, SLOT(stop()));
            } else {
                QTimer::singleShot(100, this, SLOT(wallfollowPart2()));
            }
        }
    }
}

void TaskWallFollowing::controlFinishedWaypoints(QString waypoint){
    if(this->isEnabled()){
        if(waypoint == this->getSettingsValue("taskStartPoint").toString()){
            // Task startwaypoint reached, start wallfollow
            logger->info(this->getSettingsValue("taskStartPoint").toString() +" reached");
            QTimer::singleShot(0, navi, SLOT(clearGoal()));
            QTimer::singleShot(0, this, SLOT(wallfollowPart1()));
        } else if(waypoint ==  this->getSettingsValue("wallAdjustPoint").toString()){
            // Adjust waypoint reached, start wallfollow again
            logger->info(this->getSettingsValue("wallAdjustPoint").toString() +" reached");
            QTimer::singleShot(0, navi, SLOT(clearGoal()));
            QTimer::singleShot(0, this, SLOT(wallfollowPart2()));
        } else {
            // Error!
            logger->info("Something is wrong with navigation waypoints, abort...");
            QTimer::singleShot(0, this, SLOT(emergencyStop()));
        }
    }
}

void TaskWallFollowing::controlAngleCalculation(){
    if(this->isEnabled()){
        QString currentState = this->getTaskState();
        double alpha = 0.0;
        double dist = 0.0;
        if(this->getSettingsValue("useAdjustPoint").toBool()){
            if(currentState == TASK_STATE_WALLFOLLOW_PART1){
                dist = this->navi->getDistance(this->getSettingsValue("wallAdjustPoint").toString());
                if(dist < 3){
                    flag_AdjustHeading = true;
                    logger->info("flag_AdjustHeading = true");
                }
            } else if(currentState == TASK_STATE_WALLFOLLOW_PART2){
                alpha = this->navi->getAlpha(this->getSettingsValue("goal1point1").toString(), this->getSettingsValue("goal1point2").toString());
                dist = this->navi->getDistance(this->getSettingsValue("goal1point2").toString());
                if(fabs(alpha) < 5 && dist < 2){
                    flag_GoalLine_reached = true;
                    logger->info("flag_GoalLine_reached = true");
                }
            }
        } else {
            if(currentState == TASK_STATE_WALLFOLLOW_PART1){
                alpha = this->navi->getAlpha(this->getSettingsValue("goal1point1").toString(), this->getSettingsValue("goal1point2").toString());
                dist = this->navi->getDistance(this->getSettingsValue("goal1point2").toString());
                if(fabs(alpha) < 5 && dist < 2){
                    flag_GoalLine_reached = true;
                    logger->info("flag_GoalLine_reached = true");
                }
            }
        }
        addData("Alpha", alpha);
        addData("Dist", dist);
        emit dataChanged(this);
    }
}

void TaskWallFollowing::stop(){
    if(this->isEnabled()){
        taskState = TASK_STATE_END;
        logger->info(taskState);
        addData("taskState", taskState);
        emit newState(taskState);
        emit dataChanged(this);

        running = false;
        logger->info("Taskwallfollowing stopped");

        if (this->isActive())
        {
            QTimer::singleShot(0, navi, SLOT(clearGoal()));
            this->taskTimer.stop();
            QTimer::singleShot(0, wall, SLOT(stop()));
            this->setEnabled(false);
            emit finished(this,true);
        }
    } else {
        logger->info("Something is really wrong...");
        emit finished(this,false);
    }
}

void TaskWallFollowing::timeoutStop(){
    if(this->isEnabled()){
        taskState = TASK_STATE_END_FAILED;
        logger->info(taskState);
        addData("taskState", taskState);
        emit newState(taskState);
        emit dataChanged(this);

        running = false;
        logger->info("Taskwallfollowing timeout stopped");

        if (this->isActive())
        {
            QTimer::singleShot(0, navi, SLOT(clearGoal()));
            this->taskTimer.stop();
            QTimer::singleShot(0, wall, SLOT(stop()));
            this->setEnabled(false);
            emit finished(this,false);
        }
    }
}


void TaskWallFollowing::emergencyStop(){
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
    logger->info( "Task wallfollowing emergency stopped" );

    if (this->isActive())
    {
        QTimer::singleShot(0, navi, SLOT(clearGoal()));
        this->taskTimer.stop();
        QTimer::singleShot(0, wall, SLOT(stop()));
        this->setEnabled(false);
        emit finished(this,false);
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

void TaskWallFollowing::controlEnabledChanged(bool b){
    if(b == false){
        logger->info("No longer enabled!");
        QTimer::singleShot(0, this, SLOT(emergencyStop()));
    }
}

QString TaskWallFollowing::getTaskState(){
    return taskState;
}
