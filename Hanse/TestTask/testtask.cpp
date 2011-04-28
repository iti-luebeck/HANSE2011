#include "testtask.h"
#include "testtaskform.h"
#include <QtGui>
#include <Module_Simulation/module_simulation.h>

TestTask::TestTask(QString id, Behaviour_WallFollowing *w, Module_Simulation *sim)
    : RobotBehaviour(id)
{
    qDebug()<<"testtask thread id";
    qDebug()<< QThread::currentThreadId();
    this->sim = sim;
    this->wall = w;
    this->wall->setEnabled(false);
    this->wall->updateFromSettings();

    setEnabled(false);
    running = false;

    testTimer.setSingleShot(true);
    testTimer.moveToThread(this);
}

bool TestTask::isActive(){
    return isEnabled();
}


void TestTask::init(){
    logger->debug("testtask init");
}



void TestTask::startBehaviour(){
    qDebug("Set Settings");
    this->wall->setSettingsValue("forwardSpeed",0.5);
    this->wall->setSettingsValue("angularSpeed",0.3);
    this->wall->setSettingsValue("desiredDistance",1.5);
    this->wall->setSettingsValue("corridorWidth",0.2);
    this->wall->updateFromSettings();
    addData("forwardSpeed", this->wall->getSettingsValue("forwardSpeed"));
    addData("angularSpeed", this->wall->getSettingsValue("angularSpeed"));
    addData("desiredDistance", this->wall->getSettingsValue("desiredDistance"));
    addData("corridorWidth", this->wall->getSettingsValue("corridorWidth"));
    emit dataChanged(this);

    this->reset();
    logger->info("TestTastk started" );
    running = true;
    setHealthToOk();
    setEnabled(true);
    emit started(this);
    running = true;
    //this->wall->setEnabled(true);
    this->wall->echo->setEnabled(true);
    this->wall->startBehaviour();
    countdown();
}


void TestTask::stop()
{
    running = false;
    logger->info( "testTask stopped" );

    if (this->isActive())
    {
        this->testTimer.stop();
        this->wall->stop();
        this->wall->echo->setEnabled(false);
        this->setEnabled(false);
        emit finished(this,true);
    }
}


void TestTask::emergencyStop()
{
    running = false;
    logger->info( "testTask emergency stopped" );

    if (this->isActive())
    {
        this->testTimer.stop();
        this->wall->stop();
        this->wall->echo->setEnabled(false);
        this->setEnabled(false);
        emit finished(this,false);
    }
}


void TestTask::countdown()
{
    // 1 min = 60000 msec
    int msec = 30000;
    testTimer.singleShot(msec,this, SLOT(stop()));
    addData("Tasktime msec", msec);
    emit dataChanged(this);
}

void TestTask::terminate()
{
    this->stop();
    RobotModule::terminate();
}

QWidget* TestTask::createView(QWidget *parent)
{
    return new TestTaskForm(this, parent);
}

QList<RobotModule*> TestTask::getDependencies()
{
    QList<RobotModule*> ret;
    //ret.append(tcl);
    ret.append(sim);
    return ret;
}


