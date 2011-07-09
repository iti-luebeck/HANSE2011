#include "taskgotowaypoint.h"
#include "taskgotowaypointform.h"
#include <QtGui>
#include <Module_Simulation/module_simulation.h>
#include <Module_Navigation/module_navigation.h>


TaskGotoWaypoint::TaskGotoWaypoint(QString id, Module_Simulation *sim, Module_Navigation *n)
    : RobotBehaviour(id)
{
    this->sim = sim;
    this->navi = n;
    taskTimer.moveToThread(this);
}

bool TaskGotoWaypoint::isActive(){
    return active;
}


void TaskGotoWaypoint::init(){
    logger->debug("taskgotowaypoint init");

    active = false;
    setEnabled(false);


    // Default task settings
    this->setDefaultValue("taskStopTime",5);
    this->setDefaultValue("timerActivated", true);

    // Default navigation settings
    this->setDefaultValue("point1", "p1");
    this->setDefaultValue("use1", false);
    this->setDefaultValue("point2", "p2");
    this->setDefaultValue("use2", false);
    this->setDefaultValue("point3", "p3");
    this->setDefaultValue("use3", false);
    this->setDefaultValue("point4", "p4");
    this->setDefaultValue("use4", false);
    this->setDefaultValue("point5", "p5");
    this->setDefaultValue("use5", false);
    this->setDefaultValue("point6", "p6");
    this->setDefaultValue("use6", false);

    connect(this, SIGNAL(enabled(bool)), this, SLOT(controlEnabledChanged(bool)));
    connect(navi, SIGNAL(reachedWaypoint(QString)), this, SLOT(controlFinishedWaypoints(QString)));


}

void TaskGotoWaypoint::reset() {
    RobotModule::reset();

    taskState = GTW_STATE_START;
}


void TaskGotoWaypoint::startBehaviour(){
    if (isActive()){
        logger->info("Already active!");
        return;
    }

    this->reset();


    setHealthToOk();

    taskState = GTW_STATE_START;
    showTaskState();

    active = true;
    if(!isEnabled()){
        setEnabled(true);
    }
    emit started(this);

    emit newStateOverview(GTW_STATE_MOVE_TO_WAYPOINT);

    if(!this->navi->isEnabled()){
        this->navi->setEnabled(true);
    }

    emit updateSettings();

    if(this->getSettingsValue("timerActivated").toBool()){
        logger->info("TaskGotoWaypoint with timer stop");
        int temp = this->getSettingsValue("taskStopTime").toInt()*60000;
        taskTimer.singleShot(temp,this, SLOT(timeoutStop()));
        addData("task stop time", temp);
        taskTimer.start();
    }

    taskState = GTW_STATE_MOVE_TO_WAYPOINT;
    waypointCounter = 1;
    controlTaskStates();
}

void TaskGotoWaypoint::controlTaskStates(){
    if(!isActive()){
        return;
    }

    if(taskState == GTW_STATE_MOVE_TO_WAYPOINT){
        showTaskState();
        if(waypointCounter < 7){
            if(waypointCounter == 1 && this->navi->containsWaypoint(this->getSettingsValue("point1").toString())){
                this->navi->gotoWayPoint(this->getSettingsValue("point1").toString());
            } else if(waypointCounter == 2 && this->navi->containsWaypoint(this->getSettingsValue("point2").toString())){
                this->navi->gotoWayPoint(this->getSettingsValue("point2").toString());
            } else if(waypointCounter == 3 && this->navi->containsWaypoint(this->getSettingsValue("point3").toString())){
                this->navi->gotoWayPoint(this->getSettingsValue("point3").toString());
            } else if(waypointCounter == 4 && this->navi->containsWaypoint(this->getSettingsValue("point4").toString())){
                this->navi->gotoWayPoint(this->getSettingsValue("point4").toString());
            } else if(waypointCounter == 5 && this->navi->containsWaypoint(this->getSettingsValue("point5").toString())){
                this->navi->gotoWayPoint(this->getSettingsValue("point5").toString());
            } else if(waypointCounter == 6 && this->navi->containsWaypoint(this->getSettingsValue("point6").toString())){
                this->navi->gotoWayPoint(this->getSettingsValue("point6").toString());
            } else {
                logger->error("Waypoint doesnt exists, trying next waypoint...");
                waypointCounter = waypointCounter+1;
                controlTaskStates();
            }
        } else {
            taskState == GTW_STATE_END;
            controlTaskStates();
        }
    } else {
        showTaskState();
        stop();
    }
}

void TaskGotoWaypoint::controlFinishedWaypoints(QString waypoint){
    if(!isActive()){
        return;
    }

    if(waypoint == this->getSettingsValue("point1").toString()){
        logger->info(this->getSettingsValue("point1").toString() +" reached");
        waypointCounter = waypointCounter+1;
        controlTaskStates();
    } else if(waypoint == this->getSettingsValue("point2").toString()){
        logger->info(this->getSettingsValue("point2").toString() +" reached");
        waypointCounter = waypointCounter+1;
        controlTaskStates();
    } else if(waypoint == this->getSettingsValue("point3").toString()){
        logger->info(this->getSettingsValue("point3").toString() +" reached");
        waypointCounter = waypointCounter+1;
        controlTaskStates();
    } else if(waypoint == this->getSettingsValue("point4").toString()){
        logger->info(this->getSettingsValue("point4").toString() +" reached");
        waypointCounter = waypointCounter+1;
        controlTaskStates();
    } else if(waypoint == this->getSettingsValue("point5").toString()){
        logger->info(this->getSettingsValue("point5").toString() +" reached");
        waypointCounter = waypointCounter+1;
        controlTaskStates();
    } else if(waypoint == this->getSettingsValue("point6").toString()){
        logger->info(this->getSettingsValue("point6").toString() +" reached");
        waypointCounter = waypointCounter+1;
        controlTaskStates();
    } else {
        // None of current task waypoints reached oO
        logger->info("Something is wrong with navigation waypoints, abort...");
        emergencyStop();;
    }
}


void TaskGotoWaypoint::showTaskState(){
    logger->info(taskState);
    emit newState(this->getId(), taskState);

    addData("taskState", taskState);
    emit dataChanged(this);
}

void TaskGotoWaypoint::stop(){
    if(!isActive()){
        logger->info("Not active!");
        return;
    }

    logger->info("Taskgotowaypoint stopped");
    taskTimer.stop();
    active = false;
    setEnabled(false);
    taskState = GTW_STATE_END;
    showTaskState();

    if(navi->hasGoal()){
        QTimer::singleShot(0, navi, SLOT(clearGoal()));
    }

    emit finished(this,true);
}

void TaskGotoWaypoint::timeoutStop(){
    if(!isActive()){
        logger->info("Not active!");
        return;
    }

    logger->info("Taskgotowaypoint timeout stopped");
    taskTimer.stop();
    active = false;
    setEnabled(false);
    taskState = GTW_STATE_END;
    showTaskState();

    if(navi->hasGoal()){
        QTimer::singleShot(0, navi, SLOT(clearGoal()));
    }

    emit finished(this,false);
}


void TaskGotoWaypoint::emergencyStop(){
    if(!isActive()){
        logger->info("Not , no emergency stop needed");
        return;
    }

    logger->info("Taskgotowaypoint stopped");
    taskTimer.stop();
    active = false;
    setEnabled(false);
    taskState = GTW_STATE_END;
    showTaskState();

    if(navi->hasGoal()){
        QTimer::singleShot(0, navi, SLOT(clearGoal()));
    }

    emit finished(this,false);
}


void TaskGotoWaypoint::terminate()
{
    this->stop();
    RobotModule::terminate();
}

QWidget* TaskGotoWaypoint::createView(QWidget *parent)
{
    return new TaskGotoWaypointForm(this, parent);
}

QList<RobotModule*> TaskGotoWaypoint::getDependencies()
{
    QList<RobotModule*> ret;
    ret.append(navi);
    ret.append(sim);
    return ret;
}

void TaskGotoWaypoint::controlEnabledChanged(bool enabled){
    if(!enabled && isActive()){
        logger->info("Disable and deactivate TaskGotoWaypoint");
        stop();
    } else if(!enabled && !isActive()){
        //logger->info("Still deactivated");
    } else if(enabled && !isActive()){
        //logger->info("Enable and activate TaskGotoWaypoint");
        //startBehaviour();
    } else {
        //logger->info("Still activated");
    }
}

QString TaskGotoWaypoint::getTaskState(){
    return taskState;
}
