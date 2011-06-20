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
    this->setDefaultValue("pipeNavStart", "pS");
    this->setDefaultValue("pipeNavEnd1", "pE1");
    this->setDefaultValue("pipeNavEnd2", "pE2");
    this->setDefaultValue("targetTolerance", 10);
    this->setDefaultValue("useStartNavigation", true);
    this->setDefaultValue("timerActivated", true);
    this->setDefaultValue("loopActivated", true);


    // Default turn180 settings
    this->setDefaultValue("hysteresis", 10);
    this->setDefaultValue("p", 0.4);
    this->setDefaultValue("degree", 180);

    taskTimer.setSingleShot(true);
    taskTimer.moveToThread(this);

    distanceToPipeEnd = 0;

    connect(this, SIGNAL(enabled(bool)), this, SLOT(controlEnabledChanged(bool)));
    connect(navi, SIGNAL(reachedWaypoint(QString)), this, SLOT(seReached(QString)));
    connect(turn180, SIGNAL(turn180finished(QString)), this, SLOT(seReached(QString)));
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
    logger->info("Taskpipefollowing started" );
    running = true;
    setHealthToOk();
    if(!this->isEnabled()){
        if(this->getSettingsValue("useStartNavigation").toBool() == true){
            emit newStateOverview("Move to start");
        }
        emit newStateOverview("Do pipefollowing");
        emit newStateOverview("until end of pipe reached");
        emit newStateOverview("Do turn180");
        emit newStateOverview("Do pipefollowing");
        emit newStateOverview("until end of pipe reached");
        if(this->getSettingsValue("loopActivated").toBool()){
            emit newStateOverview("...in a loop");
        }
    }

    setEnabled(true);
    emit started(this);
    running = true;

    // Enable all components
    if(!this->navi->isEnabled()){
        this->navi->setEnabled(true);
    }

    addData("loop", this->getSettingsValue("loopActivated").toBool());
    addData("stoptimer", this->getSettingsValue("timerActivated").toBool());
    addData("state", "Start TaskPipeFollowing");
    emit dataChanged(this);
    emit newState("Start TaskPipeFollowing");

    emit updateSettings();


    if(this->getSettingsValue("timerActivated").toBool()){
        logger->debug("TaskPipeFollowing with timer stop");
        taskTimer.singleShot(this->getSettingsValue("taskStopTime").toInt(),this, SLOT(timeoutStop()));
        taskTimer.start();
    }

    QTimer::singleShot(0, this, SLOT(moveToStart()));
}


void TaskPipeFollowing::moveToStart(){
    if(this->isEnabled() && this->navi->getHealthStatus().isHealthOk()){
        if(this->getSettingsValue("useStartNavigation").toBool() == false){
            logger->info("Move to start");
            addData("state", "Move to start");
            emit newState("Move to start");
            emit dataChanged(this);
            this->navi->gotoWayPoint(this->getSettingsValue("pipeNavStart").toString());
        } else {
            QTimer::singleShot(0, this, SLOT(doPipeFollow()));
        }
    } else {
        logger->info("Something is wrong with navigation/task, abort...");
        QTimer::singleShot(0, this, SLOT(emergencyStop()));
    }
}


void TaskPipeFollowing::seReached(QString waypoint){
    if(this->isEnabled()){
        if(waypoint == this->getSettingsValue("pipeNavStart").toString()){
            // Start reached, do pipefollowing
            logger->info("Start reached");
            QTimer::singleShot(0, navi, SLOT(clearGoal()));
            QTimer::singleShot(0, this, SLOT(doPipeFollow()));
        } else if(waypoint == "turn180"){
            // Turn180 finished, move to end
            logger->info("Turn180 finished!");
            QTimer::singleShot(0, this, SLOT(doPipeFollow()));
        } else {
            // Error!
            logger->info("Something is wrong with navigation waypoints, abort...");
            QTimer::singleShot(0, this, SLOT(emergencyStop()));
        }
    }
}

void TaskPipeFollowing::doPipeFollow(){
    if(this->isEnabled()){
        logger->info("Do pipefollowing");
        addData("state", "Do pipefollowing");
        emit newState("Do pipefollowing");
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

        // Now do pipefollowing until target position is reached
        QTimer::singleShot(0, this, SLOT(controlPipeFollowRemainingDistance()));

    } else {
        logger->info("Not enabled!");
        QTimer::singleShot(0, this, SLOT(emergencyStop()));
    }
}

void TaskPipeFollowing::controlPipeFollowRemainingDistance(){
    if(this->isEnabled()){
        if(counter == 1){
            // First pipe end
            distanceToPipeEnd = this->navi->getDistance(this->getSettingsValue("pipeNavEnd1").toString());
        } else if(counter == 2){
            // Second pipe end
            distanceToPipeEnd = this->navi->getDistance(this->getSettingsValue("pipeNavEnd2").toString());
        } else {
            logger->info("controlPipeFollowRemainingDistance error!");
            QTimer::singleShot(0, this, SLOT(emergencyStop()));
        }

        logger->debug("remaining distance "+QString::number(distanceToPipeEnd));
        addData("remaining distance",distanceToPipeEnd);
        emit dataChanged(this);
        if(this->pipe->getHealthStatus().isHealthOk() == true){
            if(this->distanceToPipeEnd > this->getSettingsValue("targetTolerance").toDouble()){
                QTimer::singleShot(200, this, SLOT(controlPipeFollowRemainingDistance()));
            } else {
                addData("remaining distance", "-");
                emit dataChanged(this);
                addData("state", "Pipefollowing finished");
                emit dataChanged(this);

                // Target position reached, stop pipefollow
                if(this->pipe->isEnabled()){
                    logger->debug("Target position reached; disable pipefollow");
                    QTimer::singleShot(0, pipe, SLOT(stop()));
                }

                if(counter == 1){
                    // First end, do turn
                    QTimer::singleShot(0, this, SLOT(doTurn()));
                } else {
                    // Second end
                    QTimer::singleShot(0, this, SLOT(controlNextState()));
                }
            }
        } else {
            logger->info("Pipe health is not ok...");
            QTimer::singleShot(0, this, SLOT(emergencyStop()));
        }
    } else {
        logger->info("Not enabled!");
        QTimer::singleShot(0, this, SLOT(emergencyStop()));
    }
}


void TaskPipeFollowing::doTurn(){
    if(this->isEnabled()){
        logger->debug("turn180");
        addData("state", "Turn180");
        emit dataChanged(this);
        emit newState("Turn180");

        if(!this->turn180->isEnabled()){
            logger->debug("enable turn180");
            this->turn180->setSettingsValue("hysteresis", this->getSettingsValue("hysteresis").toFloat());
            this->turn180->setSettingsValue("p", this->getSettingsValue("p").toFloat());
            this->turn180->setSettingsValue("degree", this->getSettingsValue("degree").toFloat());
            QTimer::singleShot(0, turn180, SLOT(startBehaviour()));
        }
    } else {
        logger->info("doTurn not possible!");
        QTimer::singleShot(0, this, SLOT(moveToEnd()));
    }

}



void TaskPipeFollowing::moveToEnd(){
    if(this->isEnabled() && this->navi->getHealthStatus().isHealthOk()){
        logger->info("Move to end");
        addData("state", "Move to end");
        emit dataChanged(this);
        emit newState("Move to end");

        this->navi->gotoWayPoint(this->getSettingsValue("startNavigation").toString());
    } else {
        logger->info("Something is wrong with navigation, abort...");
        QTimer::singleShot(0, this, SLOT(emergencyStop()));
    }
}


void TaskPipeFollowing::controlNextState(){
    if(this->isEnabled()){
        logger->debug("idle, controlNextState");
        addData("state", "Idle, control next state");
        emit dataChanged(this);
        emit newState("Idle, control next state");

        if(this->getSettingsValue("loopActivated").toBool()){
            logger->debug("start again");
            QTimer::singleShot(0, this, SLOT(moveToStart()));
        } else {
            // Task finished, stop
            QTimer::singleShot(0, this, SLOT(stop()));
        }
    } else {
        logger->info("Something is wrong with the task, abort...");
        QTimer::singleShot(0, this, SLOT(emergencyStop()));
    }
}

void TaskPipeFollowing::stop(){
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
