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
}

bool TestTask::isActive()
{
    return isEnabled();
}


void TestTask::init()
{
    logger->debug("testtask init");
    //timer.moveToThread(this);
    testTimer = new QTimer(this);


}

void TestTask::startBehaviour()
{
    this->reset();
    logger->info("TestTastk started" );
    running = true;
    this->setHealthToOk();
    this->setEnabled(true);
    emit started(this);
    running = true;
    //this->wall->setEnabled(true);
    this->wall->startBehaviour();
    countdown();
}


void TestTask::stop()
{
    running = false;
logger->info( "testTask stopped" );

    if (this->isActive())
    {

        this->wall->stop();
        this->setEnabled(false);
        emit finished(this,false);
    }
}



void TestTask::countdown()
{
    qDebug("Testtask: start countdown");
    // 1 min = 60000 msec
    //estTimer->start(60000);
    //testTimer->singleShot(60000,this,SLOT(stop()));
    stop();
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

