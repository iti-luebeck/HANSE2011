#include "tasksurface.h"
#include "tasksurfaceform.h"
#include <QtGui>
#include <Module_Simulation/module_simulation.h>
#include <Module_Navigation/module_navigation.h>
#include <Behaviour_PingerFollowing/behaviour_pingerfollowing.h>
#include <Behaviour_XsensFollowing/behaviour_xsensfollowing.h>


TaskSurface::TaskSurface(QString id, Module_Simulation *sim, Module_Navigation *n, Behaviour_PingerFollowing *pf, Behaviour_XsensFollowing *xf)
    : RobotBehaviour(id)
{
    this->sim = sim;
    this->navi = n;
    this->xsensfollow = xf;
    this->pingerfollow = pf;

    taskTimer.moveToThread(this);

}

bool TaskSurface::isActive(){
    return active;
}


void TaskSurface::init(){
    logger->debug("tasksurface init");

    this->setDefaultValue("taskStopTime",120000);
    this->setDefaultValue("timerActivated", true);

    this->setDefaultValue("taskStartPoint", "mid start");


    active = false;
    setEnabled(false);

    connect(this, SIGNAL(enabled(bool)), this, SLOT(controlEnabledChanged(bool)));
    connect(navi, SIGNAL(reachedWaypoint(QString)), this, SLOT(controlFinishedWaypoints(QString)));
    connect(xsensfollow, SIGNAL(newXsensState(QString)), this, SLOT(controlXsensState(QString)));
    //connect(pingerfollow, SIGNAL(newPingerState(QString)), this, SLOT(controlPingerState(QString)));

    reset();
}

void TaskSurface::reset() {
    RobotModule::reset();
}


void TaskSurface::startBehaviour(){
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

    taskState = SURFACE_STATE_START;
    showTaskState();

    emit updateSettings();

    if(this->getSettingsValue("timerActivated").toBool()){
        logger->info("TaskSurface with timer stop");
        int temp = this->getSettingsValue("taskStopTime").toInt()*60000;
        taskTimer.singleShot(temp,this, SLOT(timeoutStop()));
        addData("task stop time", temp);
        emit dataChanged(this);
        taskTimer.start();
    }

    taskState = SURFACE_STATE_MOVE_TO_TASK_START;
    controlTaskStates();
}


void TaskSurface::controlTaskStates(){
    if (taskState == SURFACE_STATE_MOVE_TO_TASK_START) {

        showTaskState();
        this->navi->gotoWayPoint(this->getSettingsValue("taskStartPoint").toString());

    }  else if(taskState == SURFACE_STATE_PINGERFOLLOWING){
        showTaskState();
        if(this->pingerfollow->getHealthStatus().isHealthOk()){
            if (!this->pingerfollow->isActive()) {
                logger->info("Activate pingerfollowing");
                QTimer::singleShot(0, pingerfollow, SLOT(startBehaviour()));
            } else {
                QTimer::singleShot(0, pingerfollow, SLOT(reset()));
            }
        } else {
            logger->info("Pingerfollowing not possible");
            //taskState = SURFACE_STATE_XSENSFOLLOW;
            controlTaskStates();
        }

    } else if(taskState == SURFACE_STATE_END){

        showTaskState();
        stop();

    }
}


void TaskSurface::controlFinishedWaypoints(QString waypoint) {

    if(!isActive()){
        return;
    }

    if (taskState == SURFACE_STATE_MOVE_TO_TASK_START) {
        if (waypoint == this->getSettingsValue("taskStartPoint").toString()) {
            logger->info(this->getSettingsValue("taskStartPoint").toString() +" reached");
            taskState = SURFACE_STATE_FIND_PING;
            controlTaskStates();
        } else {
            navi->gotoWayPoint(this->getSettingsValue("taskStartPoint").toString());
        }
    }


}

void TaskSurface::controlPingerState(QString newState){

    if (!isActive()) {
        return;
    }

    Q_UNUSED(newState);

    if (taskState == SURFACE_STATE_PINGERFOLLOWING) {

    }


}


void TaskSurface::controlXsensState(QString newState){
    if(!isActive()){
        return;
    }
    Q_UNUSED(newState);
}

void TaskSurface::initBehaviourParameters(){


}

void TaskSurface::showTaskState(){
    logger->info(taskState);
    emit newState(this->getId(), taskState);
    addData("taskState", taskState);
    emit dataChanged(this);
}


void TaskSurface::stop(){
    if(!isActive()){
        logger->info("Not active!");
        return;
    }

    logger->info("TaskSurface stopped" );

    active = false;
    setEnabled(false);

    taskState = SURFACE_STATE_END;
    showTaskState();

    if(navi->hasGoal()){
        QTimer::singleShot(0, navi, SLOT(clearGoal()));
    }

    taskTimer.stop();
    emit finished(this,true);
}

void TaskSurface::timeoutStop(){
    if(!isActive()){
        logger->info("Not active!");
        return;
    }

    logger->info("TaskSurface timeout stopped" );

    active = false;
    setEnabled(false);

    taskState = SURFACE_STATE_END_FAILED;
    showTaskState();

    if(navi->hasGoal()){
        QTimer::singleShot(0, navi, SLOT(clearGoal()));
    }

    taskTimer.stop();
    emit finished(this,false);

}



void TaskSurface::emergencyStop(){
    if(!isActive()){
        logger->info("Not active, no emergency stop needed");
        return;
    }

    logger->info("TaskSurface emergency stopped" );

    active = false;
    setEnabled(false);

    taskState = SURFACE_STATE_END_FAILED;
    showTaskState();

    if(navi->hasGoal()){
        QTimer::singleShot(0, navi, SLOT(clearGoal()));
    }

    taskTimer.stop();
    emit finished(this,false);

}


void TaskSurface::terminate()
{
    this->stop();
    RobotModule::terminate();
}

QWidget* TaskSurface::createView(QWidget *parent)
{
    return new TaskSurfaceForm(this, parent);
}

QList<RobotModule*> TaskSurface::getDependencies()
{
    QList<RobotModule*> ret;
    ret.append(navi);
    ret.append(sim);
    return ret;
}

void TaskSurface::controlEnabledChanged(bool enabled){
    if(!enabled && isActive()){
        logger->info("Disable and deactivate TaskSurface");
        stop();
    } else if(!enabled && !isActive()){
        //logger->info("Still deactivated");
    } else if(enabled && !isActive()){
        //logger->info("Enable and activate TaskSurface");
        //startBehaviour();
    } else {
        //logger->info("Still activated");
    }
}

QString TaskSurface::getTaskState()
{
    return taskState;
}
