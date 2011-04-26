/*

   Dieser Task führt für eine Minute ein Wandverfolgungsverhalten mit veränderten Parametern aus.
   Zusätzlich wird aufgetaucht.

*/


#include "testtask2.h"
#include "testtask2form.h"
#include <QtGui>
#include <Module_Simulation/module_simulation.h>
#include <Module_ThrusterControlLoop/module_thrustercontrolloop.h>

TestTask2::TestTask2(QString id, Behaviour_WallFollowing *w, Module_Simulation *sim, Module_ThrusterControlLoop* t)
    : RobotBehaviour(id)
{
    qDebug()<<"testtask2 thread id";
    qDebug()<< QThread::currentThreadId();
    this->tcl = t;
    this->sim = sim;
    this->wall = w;
    this->wall->setEnabled(false);


    connect(this,SIGNAL(setDepth(float)),tcl,SLOT(setDepth(float)));

    setEnabled(false);
    running = false;

    testTimer.setSingleShot(true);
    testTimer.moveToThread(this);
}

bool TestTask2::isActive()
{
    return isEnabled();
}


void TestTask2::init()
{
    logger->debug("testtask2 init");
    //timer.moveToThread(this);
    //testTimer = new QTimer(this);


}

void TestTask2::startBehaviour()
{
    emit setDepth(0);
    this->wall->setSettingsValue("forwardSpeed",0.5);
    this->wall->setSettingsValue("angularSpeed",0.3);
    this->wall->setSettingsValue("desiredDistance",3.0);
    this->wall->setSettingsValue("corridorWidth",0.1);
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



void TestTask2::stop()
{
    running = false;
    logger->info( "testTask2 stopped" );

    if (this->isActive())
    {
        this->testTimer.stop();
        this->wall->stop();
        this->wall->echo->setEnabled(false);
        this->setEnabled(false);
        emit finished(this,true);

    }
}


void TestTask2::emergencyStop()
{
    running = false;
    logger->info( "testTask2 emergency stopped" );

    if (this->isActive())
    {
        this->testTimer.stop();
        this->wall->stop();
        this->wall->echo->setEnabled(false);
        this->setEnabled(false);
        emit finished(this,false);
    }
}


void TestTask2::countdown()
{
    //qDebug("Testtask2: start countdown");
    // 1 min = 60000 msec
    int msec = 30000;
    testTimer.singleShot(msec,this, SLOT(stop()));
    addData("Tasktime msec", msec);
    emit dataChanged(this);
    //stop();
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


