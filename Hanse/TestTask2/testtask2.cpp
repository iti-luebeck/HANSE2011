#include "testtask2.h"
#include "testtask2form.h"
#include <QtGui>
#include <Module_Simulation/module_simulation.h>

TestTask2::TestTask2(QString id, Behaviour_WallFollowing *w, Module_Simulation *sim)
    : RobotBehaviour(id)
{
    qDebug()<<"testtask2 thread id";
    qDebug()<< QThread::currentThreadId();
    this->sim = sim;
    this->wall = w;
    this->wall->setEnabled(false);

    setEnabled(false);
    running = false;
}

bool TestTask2::isActive()
{
    return isEnabled();
}


void TestTask2::init()
{
    logger->debug("testtask init");
    //timer.moveToThread(this);
    testTimer = new QTimer(this);


}

void TestTask2::startBehaviour()
{
    this->reset();
    logger->info("TestTask2 started" );
    running = true;
    this->setHealthToOk();
    this->setEnabled(true);
    emit started(this);
    running = true;
    //this->wall->setEnabled(true);
    this->wall->startBehaviour();
    countdown();
}


void TestTask2::stop()
{
    running = false;


    if (this->isActive())
    {
        logger->info( "testTask2 stopped" );
        this->wall->stop();
        this->setEnabled(false);
        emit finished(this,false);
    }
}



void TestTask2::countdown()
{
    qDebug("Testtask2: start countdown");
    // 1 min = 60000 msec
    //estTimer->start(60000);
    //testTimer->singleShot(60000,this,SLOT(stop()));
    stop();
}

void TestTask2::terminate()
{
    this->stop();
    RobotModule::terminate();
}

void TestTask2::reset()
{


}

QWidget* TestTask2::createView(QWidget *parent)
{
    return new TestTask2Form(this, parent);
}

QList<RobotModule*> TestTask2::getDependencies()
{
    QList<RobotModule*> ret;
    //ret.append(tcl);
    ret.append(sim);
    return ret;
}

