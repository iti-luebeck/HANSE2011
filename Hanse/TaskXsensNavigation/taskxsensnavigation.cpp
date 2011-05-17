#include "taskxsensnavigation.h"
#include "taskxsensnavigationform.h"
#include <QtGui>
#include <Module_Simulation/module_simulation.h>

TaskXsensNavigation::TaskXsensNavigation(QString id, Module_Simulation *sim, Behaviour_XsensFollowing *xf, Module_Navigation *n)
    : RobotBehaviour(id)
{
    this->sim = sim;
    this->xsensfollow = xf;
    this->navi = n;

    setEnabled(false);
    running = false;

    // Default settings
    this->setDefaultValue("forwardSpeed",0.5);
    this->setDefaultValue("angularSpeed",0.3);
    this->setDefaultValue("desiredDistance",1.5);
    this->setDefaultValue("corridorWidth",0.2);
    this->setDefaultValue("description", "");
    this->setDefaultValue("taskStopTime",120000);
    this->setDefaultValue("signalTimer",20000);


    this->setDefaultValue("startNavigation", "startN");
    this->setDefaultValue("startTolerance", 10);
    this->setDefaultValue("targetNavigation", "targetN");
    this->setDefaultValue("targetTolerance", 10);

    this->setDefaultValue("timerActivated", true);
    this->setDefaultValue("loopActivated", true);

    taskTimer.setSingleShot(true);
    taskTimer.moveToThread(this);

    distanceToStart = 0;
    distanceToTarget = 0;


    connect(this, SIGNAL(stopSignal()), this, SLOT(stop()));

    connect(this, SIGNAL(moveToStartSignal()), this, SLOT(moveToStartSlot()));
    connect(this, SIGNAL(moveToEndSignal()), this, SLOT(moveToEndSlot()));
    connect(this, SIGNAL(doXsensFollowSignal()), this, SLOT(doXsensFollowSlot()));
    connect(this, SIGNAL(controlNextStateSignal()), this, SLOT(controlNextStateSlot()));

    taskTimer.moveToThread(this);
    moveToStartTimer.moveToThread(this);
    moveToEndTimer.moveToThread(this);
    doXsensFollowTimer.moveToThread(this);
    controlNextStateTimer.moveToThread(this);
}

bool TaskXsensNavigation::isActive(){
    return isEnabled();
}


void TaskXsensNavigation::init(){
    logger->debug("taskxsensnavigation init");
}



void TaskXsensNavigation::startBehaviour(){
    this->reset();
    logger->info("Taskxsensnavigation started" );
    running = true;
    setHealthToOk();
    setEnabled(true);
    emit started(this);
    running = true;


    // Enable all components
    if(!this->navi->isEnabled()){
        this->navi->setEnabled(true);
    }

    addData("loop", this->getSettingsValue("loopActivated").toBool());
    addData("timer", this->getSettingsValue("timerActivated").toBool());
    addData("state", "start TaskXsensNavigation");
    emit dataChanged(this);

    emit updateSettings();

    if(this->getSettingsValue("timerActivated").toBool()){
        logger->debug("Taskxsensnavigation with timer stop");
        taskTimer.singleShot(this->getSettingsValue("taskStopTime").toInt(),this, SLOT(timeoutStop()));
        taskTimer.start();
    }

    emit moveToStartSignal();

}

void TaskXsensNavigation::moveToStartSlot(){
    if(this->isEnabled()){
        logger->debug("move to start");
        addData("state", "Move to start");
        emit dataChanged(this);

        // First navigate to start position
        distanceToStart = this->navi->getDistance(this->getSettingsValue("startNavigation").toString());
        if(distanceToStart > this->getSettingsValue("startTolerance").toDouble()){
            addData("remaining distance", distanceToStart);
            emit dataChanged(this);
            moveToStartTimer.singleShot(this->getSettingsValue("signalTimer").toInt(),this, SLOT(moveToStartSlot()));
        } else {
            addData("remaining distance", "-");
            emit dataChanged(this);
            addData("state", "Startposition reached");
            emit dataChanged(this);
            emit doXsensFollowSignal();
        }
    }
}
void TaskXsensNavigation::doXsensFollowSlot(){
    if(this->isEnabled()){
        logger->debug("Do xsensfollowing");
        addData("state", "Do xsensfollowing");
        emit dataChanged(this);
        //this->xsensfollow->setSettingsValue("corridorWidth",this->getSettingsValue("corridorWidth").toFloat());
       // this->xsensfollow->setSettingsValue("desiredDistance",this->getSettingsValue("desiredDistance").toFloat());
        if(!this->xsensfollow->isEnabled()){
            //logger->debug("enable xsensfollow and echo");
            QTimer::singleShot(0, xsensfollow, SLOT(startBehaviour()));
        }

        // Now do xsensfollowing until target position is reached
        distanceToTarget = this->navi->getDistance(this->getSettingsValue("targetNavigation").toString());
        if(distanceToTarget > this->getSettingsValue("targetTolerance").toDouble()){
            addData("remaining distance",distanceToTarget);
            emit dataChanged(this);

            // Too many errors, abort xsenslfollowing and move to endposition
            //if(this->wall->badDataCount > 100){
           //     emit moveToEndSignal();
            //} else {
                doXsensFollowTimer.singleShot(this->getSettingsValue("signalTimer").toInt(),this, SLOT(doXsensFollowSlot()));
            //}
        } else {
            addData("remaining distance", "-");
            emit dataChanged(this);
            addData("state", "Xsensfollowing finished");
            emit dataChanged(this);

            // Target position reached, stop xsensfollow
            if(this->xsensfollow->isEnabled()){
                logger->debug("Disable xsensfollow and echo");

                QTimer::singleShot(0, xsensfollow, SLOT(stop()));
                //this->wall->echo->setEnabled(false);
            }

            emit controlNextStateSignal();
        }
    }
}

void TaskXsensNavigation::moveToEndSlot(){
    if(this->isEnabled()){
        logger->debug("Move to end");
        addData("state", "Move to end");
        emit dataChanged(this);

        distanceToTarget = this->navi->getDistance(this->getSettingsValue("targetNavigation").toString());
        if(distanceToTarget > this->getSettingsValue("targetTolerance").toDouble()){
            addData("remaining distance", distanceToTarget);
            emit dataChanged(this);
            moveToEndTimer.singleShot(this->getSettingsValue("signalTimer").toInt(),this, SLOT(moveToEndSlot()));
        } else {
            emit controlNextStateSignal();
        }
    }
}

void TaskXsensNavigation::controlNextStateSlot(){
    if(this->isEnabled()){
        logger->debug("idle, controlNextState");
        addData("state", "Idle, control next state");
        emit dataChanged(this);

        if(this->getSettingsValue("loopActivated").toBool()){
            logger->debug("start again");
            emit moveToStartSignal();
        } else {
            // Task finished, stop
            emit stopSignal();
        }
    }
}


void TaskXsensNavigation::stop(){
    if(this->isEnabled()){
        running = false;
        logger->info("Taskxsensnavigation stopped");

        if (this->isActive())
        {
            this->taskTimer.stop();

            this->navi->setEnabled(false);
           // this->wall->echo->setEnabled(false);
            QTimer::singleShot(0, xsensfollow, SLOT(stop()));

            this->setEnabled(false);
            emit finished(this,true);
        }
    }
}

void TaskXsensNavigation::timeoutStop(){
    if(this->isEnabled()){running = false;
        logger->info("Taskxsensnavigation timeout stopped");

        if (this->isActive())
        {
            this->taskTimer.stop();

            this->navi->setEnabled(false);
            //this->wall->echo->setEnabled(false);
            QTimer::singleShot(0, xsensfollow, SLOT(stop()));

            this->setEnabled(false);
            emit finished(this,false);
        }
    }
}

void TaskXsensNavigation::emergencyStop(){
    running = false;
    logger->info( "Taskxsensnavigation emergency stopped" );

    if (this->isActive())
    {
        this->taskTimer.stop();
        this->navi->setEnabled(false);
       // this->wall->echo->setEnabled(false);
        QTimer::singleShot(0, xsensfollow, SLOT(stop()));
        this->setEnabled(false);
        emit finished(this,false);
    }
}


void TaskXsensNavigation::terminate()
{
    this->stop();
    RobotModule::terminate();
}

QWidget* TaskXsensNavigation::createView(QWidget *parent)
{
    return new TaskXsensNavigationForm(this, parent);
}

QList<RobotModule*> TaskXsensNavigation::getDependencies()
{
    QList<RobotModule*> ret;
    ret.append(xsensfollow);
    ret.append(sim);
    return ret;
}


