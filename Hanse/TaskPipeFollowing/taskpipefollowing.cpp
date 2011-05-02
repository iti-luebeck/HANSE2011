#include "taskpipefollowing.h"
#include "taskpipefollowingform.h"
#include <QtGui>
#include <Module_Simulation/module_simulation.h>

TaskPipeFollowing::TaskPipeFollowing(QString id, Behaviour_PipeFollowing *w, Module_Simulation *sim)
    : RobotBehaviour(id)
{
    qDebug()<<"testtask thread id";
    qDebug()<< QThread::currentThreadId();
    this->sim = sim;
    this->pipe = w;
    this->pipe->setEnabled(false);

    setEnabled(false);
    running = false;

    // Default settings

//    this->timerTime = this->getSettingsValue("timer",0).toInt();
//    this->threshSegmentation = this->getSettingsValue("threshold",188).toInt();
//    this->debug = this->getSettingsValue("debug",0).toInt();
//    this->deltaAngPipe = this->getSettingsValue("deltaAngle",11).toFloat();
//    this->deltaDistPipe = this->getSettingsValue("deltaDist",100).toFloat();
//    this->kpDist = this->getSettingsValue("kpDist",1).toFloat();
//    this->kpAngle = this->getSettingsValue("kpAngle",1).toFloat();
//    this->constFWSpeed = this->getSettingsValue("fwSpeed",0.8).toFloat();
//    this->robCenter = Point(this->getSettingsValue("robCenterX",320).toDouble(),this->getSettingsValue("robCenterY",240).toDouble());
//    this->maxDistance = this->getSettingsValue("maxDistance",320).toFloat();


//    this->setDefaultValue("forwardSpeed1",0.5);
//    this->setDefaultValue("angularSpeed1",0.3);
//    this->setDefaultValue("desiredDistance1",1.5);
//    this->setDefaultValue("corridorWidth1",0.2);





    this->setDefaultValue("taskDuration1",30000);
    this->setDefaultValue("description1", "task 1");




    this->setDefaultValue("taskDuration2",30000);
    this->setDefaultValue("description2", "task 2");



    this->setDefaultValue("taskDuration3",30000);
    this->setDefaultValue("description3", "task 3");



    testTimer.setSingleShot(true);
    testTimer.moveToThread(this);
}

bool TaskPipeFollowing::isActive(){
    return isEnabled();
}


void TaskPipeFollowing::init(){
    logger->debug("testtask init");
}



void TaskPipeFollowing::startBehaviour(){
    this->reset();
    logger->info("TestTastk started" );
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
   if(taskNr == 3){
//    this->timerTime = this->getSettingsValue("timer",0).toInt();
//    this->threshSegmentation = this->getSettingsValue("threshold",188).toInt();
//    this->debug = this->getSettingsValue("debug",0).toInt();
//    this->deltaAngPipe = this->getSettingsValue("deltaAngle",11).toFloat();
//    this->deltaDistPipe = this->getSettingsValue("deltaDist",100).toFloat();
//    this->kpDist = this->getSettingsValue("kpDist",1).toFloat();
//    this->kpAngle = this->getSettingsValue("kpAngle",1).toFloat();
//    this->constFWSpeed = this->getSettingsValue("fwSpeed",0.8).toFloat();
//    this->robCenter = Point(this->getSettingsValue("robCenterX",320).toDouble(),this->getSettingsValue("robCenterY",240).toDouble());
//    this->maxDistance = this->getSettingsValue("maxDistance",320).toFloat();

    } else if (taskNr == 2){

    } else {

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
    //ret.append(tcl);
    ret.append(sim);
    return ret;
}


void TaskPipeFollowing::newSchDesSlot(QString taskName,  QString newD){
    emit newSchDesSignal(taskName, newD);
}

void TaskPipeFollowing::setDescriptionSlot(){
    emit setDescriptionSignal();
}
