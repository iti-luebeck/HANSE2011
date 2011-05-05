#include "taskturn.h"
#include "taskturnform.h"
#include <QtGui>
#include <Module_Simulation/module_simulation.h>

TaskTurn::TaskTurn(QString id, Module_ThrusterControlLoop *tcl, Module_PressureSensor *ps, Module_Compass *co, Module_XsensMTi *xsens, Module_Simulation *sim)
    : RobotBehaviour(id)
{
    qDebug()<<"taskturn thread id";
    qDebug()<< QThread::currentThreadId();
    this->sim = sim;
    this->pressure = ps;
    this->compass = co;
    this->thrustercontrolloop = tcl;
    this->xsens = xsens;

    setEnabled(false);
    running = false;

    // Default settings
//    this->setDefaultValue("forwardSpeed1",0.5);
//    this->setDefaultValue("angularSpeed1",0.3);
//    this->setDefaultValue("desiredDistance1",1.5);
//    this->setDefaultValue("corridorWidth1",0.2);
//    this->setDefaultValue("taskDuration1",30000);
//    this->setDefaultValue("description1", "task 1");


    testTimer.setSingleShot(true);
    testTimer.moveToThread(this);
}

bool TaskTurn::isActive(){
    return isEnabled();
}


void TaskTurn::init(){
    logger->debug("taskturn init");
}



void TaskTurn::startBehaviour(){
    this->reset();
    logger->info("Taskturn started" );
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
//    this->wall->echo->setEnabled(true);
//    this->wall->startBehaviour();
    testTimer.singleShot(this->getSettingsValue("taskDuration").toInt(),this, SLOT(stop()));
}

void TaskTurn::setRunData(int taskNr){
//    if(taskNr == 3){
//        this->setSettingsValue("taskDuration", this->getSettingsValue("taskDuration3"));
//        this->wall->setSettingsValue("desiredDistance", this->getSettingsValue("desiredDistance3"));
//        this->wall->setSettingsValue("forwardSpeed", this->getSettingsValue("forwardSpeed3"));
//        this->wall->setSettingsValue("angularSpeed", this->getSettingsValue("angularSpeed3"));
//        this->wall->setSettingsValue("corridorWidth", this->getSettingsValue("corridorWidth3"));
//    } else if (taskNr == 2){
//        this->setSettingsValue("taskDuration", this->getSettingsValue("taskDuration2"));
//        this->wall->setSettingsValue("desiredDistance", this->getSettingsValue("desiredDistance2"));
//        this->wall->setSettingsValue("forwardSpeed", this->getSettingsValue("forwardSpeed2"));
//        this->wall->setSettingsValue("angularSpeed", this->getSettingsValue("angularSpeed2"));
//        this->wall->setSettingsValue("corridorWidth", this->getSettingsValue("corridorWidth2"));
//    } else {
//        this->setSettingsValue("taskDuration", this->getSettingsValue("taskDuration1"));
//        this->wall->setSettingsValue("desiredDistance", this->getSettingsValue("desiredDistance1"));
//        this->wall->setSettingsValue("forwardSpeed", this->getSettingsValue("forwardSpeed1"));
//        this->wall->setSettingsValue("angularSpeed", this->getSettingsValue("angularSpeed1"));
//        this->wall->setSettingsValue("corridorWidth", this->getSettingsValue("corridorWidth1"));
//    }
}

void TaskTurn::stop(){
    running = false;
    logger->info( "Task turn stopped" );

    if (this->isActive())
    {
        this->testTimer.stop();
//        this->wall->stop();
//        this->wall->echo->setEnabled(false);
        this->setEnabled(false);
        emit finished(this,true);
    }
}


void TaskTurn::emergencyStop()
{
    running = false;
    logger->info( "Task wallfollowing emergency stopped" );

    if (this->isActive())
    {

        this->setEnabled(false);
        emit finished(this,false);
    }
}


void TaskTurn::terminate()
{
    this->stop();
    RobotModule::terminate();
}

QWidget* TaskTurn::createView(QWidget *parent)
{
    return new TaskTurnForm(this, parent);
}

QList<RobotModule*> TaskTurn::getDependencies()
{
    QList<RobotModule*> ret;
    ret.append(thrustercontrolloop);
    ret.append(sim);
    ret.append(pressure);
    ret.append(xsens);
    ret.append(compass);
    return ret;
}


void TaskTurn::newSchDesSlot(QString taskName,  QString newD){
    emit newSchDesSignal(taskName, newD);
}

void TaskTurn::setDescriptionSlot(){
    emit setDescriptionSignal();
}
