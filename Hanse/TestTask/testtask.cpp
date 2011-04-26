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

    setEnabled(false);
    running = false;

    testTimer.setSingleShot(true);
    testTimer.moveToThread(this);
}

bool TestTask::isActive()
{
    return isEnabled();
}


void TestTask::init()
{
    logger->debug("testtask init");
    //timer.moveToThread(this);
    //testTimer = new QTimer(this);


}

void TestTask::startBehaviour()
{
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
    qDebug("Testtask: start countdown");
    // 1 min = 60000 msec
    testTimer.singleShot(6000,this, SLOT(stop()));

    //stop();
}

void TestTask::terminate()
{
    this->stop();
    RobotModule::terminate();
}

void TestTask::reset()
{


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

