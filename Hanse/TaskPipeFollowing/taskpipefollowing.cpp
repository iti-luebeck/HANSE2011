#include "taskpipefollowing.h"
#include "taskpipefollowingform.h"
#include <QtGui>
#include <Module_Simulation/module_simulation.h>

TaskPipeFollowing::TaskPipeFollowing(QString id, Behaviour_PipeFollowing *w, Module_Simulation *sim)
    : RobotBehaviour(id)
{
    qDebug()<<"taskpipefollowing thread id";
    qDebug()<< QThread::currentThreadId();
    this->sim = sim;
    this->pipe = w;
    this->pipe->setEnabled(false);
    connect(this,SIGNAL(setUpdatePixmapSignal(bool)),pipe,SLOT(setUpdatePixmapSlot(bool)));
    connect(this, SIGNAL(setRunDataSignal(int)), this, SLOT(setRunData(int)));

    setEnabled(false);
    running = false;

    // Default settings

    this->setDefaultValue("timer1",250);
    this->setDefaultValue("threshold1",200);
    this->setDefaultValue("debug1",20);
    this->setDefaultValue("deltaAngle1",176.0);
    this->setDefaultValue("deltaDist1",5.0);
    this->setDefaultValue("kpDist1",5.0);
    this->setDefaultValue("kpAngle1",5.0);
    this->setDefaultValue("fwSpeed1",0.3);
    this->setDefaultValue("robCenterX1",320.0);
    this->setDefaultValue("robCenterY1",176.0);
    this->setDefaultValue("maxDistance1",320);
    this->setDefaultValue("badFrames1",20);
    this->setDefaultValue("camWidth1",640);
    this->setDefaultValue("camHeight1",480);

    this->setDefaultValue("taskDuration1",30000);
    this->setDefaultValue("description1", "taskp 1");


    this->setDefaultValue("timer2",250);
    this->setDefaultValue("threshold2",200);
    this->setDefaultValue("debug2",20);
    this->setDefaultValue("deltaAngle2",176.0);
    this->setDefaultValue("deltaDist2",5.0);
    this->setDefaultValue("kpDist2",5.0);
    this->setDefaultValue("kpAngle2",5.0);
    this->setDefaultValue("fwSpeed2",0.3);
    this->setDefaultValue("robCenterX2",320.0);
    this->setDefaultValue("robCenterY2",176.0);
    this->setDefaultValue("maxDistance2",320);
    this->setDefaultValue("badFrames2",20);
    this->setDefaultValue("camWidth2",640);
    this->setDefaultValue("camHeight2",480);

    this->setDefaultValue("taskDuration2",30000);
    this->setDefaultValue("description2", "task 2");


    this->setDefaultValue("timer3",250);
    this->setDefaultValue("threshold3",200);
    this->setDefaultValue("debug3",20);
    this->setDefaultValue("deltaAngle3",176.0);
    this->setDefaultValue("deltaDist3",5.0);
    this->setDefaultValue("kpDist3",5.0);
    this->setDefaultValue("kpAngle3",5.0);
    this->setDefaultValue("fwSpeed3",0.3);
    this->setDefaultValue("robCenterX3",320.0);
    this->setDefaultValue("robCenterY3",176.0);
    this->setDefaultValue("maxDistance3",320);
    this->setDefaultValue("badFrames3",20);
    this->setDefaultValue("camWidth3",640);
    this->setDefaultValue("camHeight3",480);

    this->setDefaultValue("taskDuration3",30000);
    this->setDefaultValue("description3", "task 3");

    testTimer.setSingleShot(true);
    testTimer.moveToThread(this);
}

bool TaskPipeFollowing::isActive(){
    return isEnabled();
}


void TaskPipeFollowing::init(){
    logger->debug("taskthrustercontrol init");
}



void TaskPipeFollowing::startBehaviour(){
    this->reset();
    logger->info("Taskpipefollowing started" );
    running = true;
    setHealthToOk();
    setEnabled(true);
    emit started(this);
    running = true;
    //    addData("taskDuration", this->getSettingsValue("taskDuration"));
    //    addData("desiredDistance", this->wall->getSettingsValue("desiredDistance"));
    //    addData("forwardSpeed", this->wall->getSettingsValue("forwardSpeed"));
    //    addData("angularSpeed", this->wall->getSettingsValue("angularSpeed"));
    //    addData("corridorWidth", this->wall->getSettingsValue("corridorWidth"));
    //    emit dataChanged(this);
    //this->pipe->echo->setEnabled(true);

    this->pipe->updateFromSettings();
    this->pipe->startBehaviour();
    testTimer.singleShot(this->getSettingsValue("taskDuration").toInt(),this, SLOT(stop()));
}

void TaskPipeFollowing::setRunData(int taskNr){
    tempTask = taskNr;
    if(taskNr == 3){
        emit setUpdatePixmapSignal(this->getSettingsValue("enableUIOutput1").toBool());
        this->pipe->setSettingsValue("timer",this->getSettingsValue("timer3"));
        this->pipe->setSettingsValue("threshold",this->getSettingsValue("threshold3"));
        this->pipe->setSettingsValue("debug",this->getSettingsValue("debug3"));
        this->pipe->setSettingsValue("deltaAngle",this->getSettingsValue("deltaAngle3"));
        this->pipe->setSettingsValue("deltaDist",this->getSettingsValue("deltaDist3"));
        this->pipe->setSettingsValue("kpDist",this->getSettingsValue("kpDist3"));
        this->pipe->setSettingsValue("kpAngle",this->getSettingsValue("kpAngle3"));
        this->pipe->setSettingsValue("fwSpeed",this->getSettingsValue("fwSpeed3"));
        this->pipe->setSettingsValue("robCenterX",this->getSettingsValue("robCenterX3"));
        this->pipe->setSettingsValue("robCenterY",this->getSettingsValue("robCenterY3"));
        this->pipe->setSettingsValue("maxDistance",this->getSettingsValue("maxDistance3"));
        this->pipe->setSettingsValue("badFrames",this->getSettingsValue("badFrames3"));
        // this->pipe->setSettingsValue("camWidth",this->getSettingsValue("camWidth3"));
        // this->pipe->setSettingsValue("camHeight",this->getSettingsValue("camHeight3"));
         this->setSettingsValue("taskDuration", this->getSettingsValue("taskDuration3"));
    } else if (taskNr == 2){
        emit setUpdatePixmapSignal(this->getSettingsValue("enableUIOutput2").toBool());
        this->pipe->setSettingsValue("timer",this->getSettingsValue("timer2"));
        this->pipe->setSettingsValue("threshold",this->getSettingsValue("threshold2"));
        this->pipe->setSettingsValue("debug",this->getSettingsValue("debug2"));
        this->pipe->setSettingsValue("deltaAngle",this->getSettingsValue("deltaAngle2"));
        this->pipe->setSettingsValue("deltaDist",this->getSettingsValue("deltaDist2"));
        this->pipe->setSettingsValue("kpDist",this->getSettingsValue("kpDist2"));
        this->pipe->setSettingsValue("kpAngle",this->getSettingsValue("kpAngle2"));
        this->pipe->setSettingsValue("fwSpeed",this->getSettingsValue("fwSpeed2"));
        this->pipe->setSettingsValue("robCenterX",this->getSettingsValue("robCenterX2"));
        this->pipe->setSettingsValue("robCenterY",this->getSettingsValue("robCenterY2"));
        this->pipe->setSettingsValue("maxDistance",this->getSettingsValue("maxDistance2"));
        this->pipe->setSettingsValue("badFrames",this->getSettingsValue("badFrames2"));
        // this->pipe->setSettingsValue("camWidth",this->getSettingsValue("camWidth2"));
        // this->pipe->setSettingsValue("camHeight",this->getSettingsValue("camHeight2"));
         this->setSettingsValue("taskDuration", this->getSettingsValue("taskDuration2"));
    } else {
        emit setUpdatePixmapSignal(this->getSettingsValue("enableUIOutput3").toBool());
        this->pipe->setSettingsValue("timer",this->getSettingsValue("timer1"));
        this->pipe->setSettingsValue("threshold",this->getSettingsValue("threshold1"));
        this->pipe->setSettingsValue("debug",this->getSettingsValue("debug1"));
        this->pipe->setSettingsValue("deltaAngle",this->getSettingsValue("deltaAngle1"));
        this->pipe->setSettingsValue("deltaDist",this->getSettingsValue("deltaDist1"));
        this->pipe->setSettingsValue("kpDist",this->getSettingsValue("kpDist1"));
        this->pipe->setSettingsValue("kpAngle",this->getSettingsValue("kpAngle1"));
        this->pipe->setSettingsValue("fwSpeed",this->getSettingsValue("fwSpeed1"));
        this->pipe->setSettingsValue("robCenterX",this->getSettingsValue("robCenterX1"));
        this->pipe->setSettingsValue("robCenterY",this->getSettingsValue("robCenterY1"));
        this->pipe->setSettingsValue("maxDistance",this->getSettingsValue("maxDistance1"));
        this->pipe->setSettingsValue("badFrames",this->getSettingsValue("badFrames1"));
        // this->pipe->setSettingsValue("camWidth",this->getSettingsValue("camWidth1"));
        // this->pipe->setSettingsValue("camHeight",this->getSettingsValue("camHeight1"));
        this->setSettingsValue("taskDuration", this->getSettingsValue("taskDuration1"));
    }
}

void TaskPipeFollowing::stop(){
    running = false;
    logger->info( "Task pipefollowing stopped" );

    if (this->isActive())
    {
        this->testTimer.stop();
        this->pipe->stop();
        //this->pipe->echo->setEnabled(false);
        this->setEnabled(false);
        emit finished(this,true);
    }
}


void TaskPipeFollowing::emergencyStop()
{
    running = false;
    logger->info( "Task pipefollowing emergency stopped" );

    if (this->isActive())
    {
        this->testTimer.stop();
        this->pipe->stop();
        //this->wall->echo->setEnabled(false);
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


void TaskPipeFollowing::newSchDesSlot(QString taskName,  QString newD){
    emit newSchDesSignal(taskName, newD);
}

void TaskPipeFollowing::setDescriptionSlot(){
    emit setDescriptionSignal();
}

void TaskPipeFollowing::updateTaskSettingsSlot(){
    emit setRunDataSignal(tempTask);
    this->pipe->updateFromSettings();
}
