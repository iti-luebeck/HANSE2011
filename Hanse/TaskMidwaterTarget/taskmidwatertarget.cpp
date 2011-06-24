#include "taskmidwatertarget.h"
#include "taskmidwatertargetform.h"
#include <QtGui>
#include <Module_Simulation/module_simulation.h>
#include <Module_Navigation/module_navigation.h>


TaskMidwaterTarget::TaskMidwaterTarget(QString id, Module_Simulation *sim, Module_Navigation *n)
    : RobotBehaviour(id)
{
    this->sim = sim;
    this->navi = n;

    setEnabled(false);
    running = false;

    connect(this, SIGNAL(enabled(bool)), this, SLOT(controlEnabledChanged(bool)));
}

bool TaskMidwaterTarget::isActive(){
    return isEnabled();
}


void TaskMidwaterTarget::init(){
    logger->debug("taskmidwatertarget init");
}

void TaskMidwaterTarget::reset() {
    RobotModule::reset();

}


void TaskMidwaterTarget::startBehaviour(){
    if (this->isEnabled()){
        logger->info("Already enabled/started!");
        return;
    }
    this->reset();

    running = true;
    setHealthToOk();

    taskState = TASK_STATE_START;
    logger->info(taskState);
    addData("state", taskState);
    emit newState(taskState);
    emit dataChanged(this);

    setEnabled(true);
    emit started(this);
    running = true;


    // Enable all components
    if(!this->navi->isEnabled()){
        this->navi->setEnabled(true);
    }

    //addData("stoptimer", this->getSettingsValue("timerActivated").toBool());
    //emit dataChanged(this);


    emit updateSettings();

//    if(this->getSettingsValue("timerActivated").toBool()){
//        logger->info("TaskMidwaterTarget with timer stop");
//        taskTimer.singleShot(this->getSettingsValue("taskStopTime").toInt(),this, SLOT(timeoutStop()));
//        taskTimer.start();
//    }

}



void TaskMidwaterTarget::stop(){
    if(this->isEnabled()){
        taskState = TASK_STATE_END;
        logger->info(taskState);
        addData("state", taskState);
        emit newState(taskState);
        emit dataChanged(this);

        running = false;
        logger->info("TaskMidwaterTarget stopped");

        if (this->isActive())
        {
            QTimer::singleShot(0, navi, SLOT(clearGoal()));
            this->taskTimer.stop();
            this->setEnabled(false);
            emit finished(this,true);
        }
    } else {
        logger->info("Something is really wrong...");
        emit finished(this,false);
    }
}

void TaskMidwaterTarget::timeoutStop(){
    if(this->isEnabled()){
        taskState = TASK_STATE_END_FAILED;
        logger->info(taskState);
        addData("state", taskState);
        emit newState(taskState);
        emit dataChanged(this);

        running = false;
        logger->info("TaskMidwaterTarget timeout stopped");

        if (this->isActive())
        {
            QTimer::singleShot(0, navi, SLOT(clearGoal()));
            this->taskTimer.stop();
            this->setEnabled(false);
            emit finished(this,false);
        }
    }
}


void TaskMidwaterTarget::emergencyStop(){
    if (!this->isEnabled()){
        logger->info("Emergency stop: Not enabled!");
        return;
    }

    taskState = TASK_STATE_END_FAILED;
    logger->info(taskState);
    addData("state", taskState);
    emit newState(taskState);
    emit dataChanged(this);

    running = false;
    logger->info("TaskMidwaterTarget emergency stopped" );

    if (this->isActive())
    {
        QTimer::singleShot(0, navi, SLOT(clearGoal()));
        this->taskTimer.stop();
        this->setEnabled(false);
        emit finished(this,false);
    }
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

void TaskMidwaterTarget::controlEnabledChanged(bool b){
    if(b == false){
        logger->info("No longer enabled!");
        QTimer::singleShot(0, this, SLOT(emergencyStop()));
    }
}

QString TaskMidwaterTarget::getTaskState(){
    return taskState;
}
