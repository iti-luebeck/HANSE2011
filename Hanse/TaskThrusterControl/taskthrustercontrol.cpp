#include "taskthrustercontrol.h"
#include "taskthrustercontrolform.h"
#include <QtGui>
#include <Module_Simulation/module_simulation.h>

TaskThrusterControl::TaskThrusterControl(QString id, Module_ThrusterControlLoop *tcl, Module_Simulation *sim)
    : RobotBehaviour(id)
{
    qDebug()<<"testtask thread id";
    qDebug()<< QThread::currentThreadId();
    this->sim = sim;
    this->thrustercontrolloop = tcl;

    setEnabled(false);
    running = false;

    // Default settings
    this->setDefaultValue("forwardSpeed1",0.5);
    this->setDefaultValue("angularSpeed1",0.0);
    this->setDefaultValue("desiredDepth1",0.0);
    this->setDefaultValue("taskDuration1",10000);
    this->setDefaultValue("description1", "task 1");

    this->setDefaultValue("forwardSpeed2",0.5);
    this->setDefaultValue("angularSpeed2",0.5);
    this->setDefaultValue("desiredDepth2",0.5);
    this->setDefaultValue("taskDuration2",5000);
    this->setDefaultValue("description2", "task 2");

    this->setDefaultValue("forwardSpeed3",0.0);
    this->setDefaultValue("angularSpeed3",0.5);
    this->setDefaultValue("desiredDepth3",0.0);
    this->setDefaultValue("taskDuration3",15000);
    this->setDefaultValue("description3", "task 3");

    this->setDefaultValue("forwardSpeed4",0.5);
    this->setDefaultValue("angularSpeed4",0.0);
    this->setDefaultValue("desiredDepth4",0.5);
    this->setDefaultValue("taskDuration4",20000);
    this->setDefaultValue("description4", "task 4");

    this->setDefaultValue("forwardSpeed5",0.1);
    this->setDefaultValue("angularSpeed5",0.1);
    this->setDefaultValue("desiredDepth5",0.1);
    this->setDefaultValue("taskDuration5",10000);
    this->setDefaultValue("description5", "task 5");

    this->setDefaultValue("forwardSpeed6",0.0);
    this->setDefaultValue("angularSpeed6",0.0);
    this->setDefaultValue("desiredDepth6",0.0);
    this->setDefaultValue("taskDuration6",10000);
    this->setDefaultValue("description6", "task 6");

    testTimer.setSingleShot(true);
    testTimer.moveToThread(this);


    /**
      * Set forward speed and hold it until told otherwise.
      *
      * range of speed: -1.0 to 1.0
      */
    connect(this,SIGNAL(forwardSpeed(float)),thrustercontrolloop,SLOT(setForwardSpeed(float)));

    /**
      * Set angular speed and hold it until told otherwise.
      *
      * positive angular speed implies a clockwise rotation;
      * negative angular speed implies a counterclockwise rotation.
      * (both rotations as seem from above the robot)
      *
      * range: -1.0 to 1.0
      */
    connect(this,SIGNAL(angularSpeed(float)),thrustercontrolloop,SLOT(setAngularSpeed(float)));

    /**
      * Dive to "depth" and stay there.
      *
      * depth: meters below the surface
      *        range: 0 to infinity
      */
    connect(this,SIGNAL(setDepth(float)),thrustercontrolloop,SLOT(setDepth(float)));
}

bool TaskThrusterControl::isActive(){
    return isEnabled();
}


void TaskThrusterControl::init(){
    logger->debug("testtask init");
}



void TaskThrusterControl::startBehaviour(){
    this->reset();
    logger->info("TaskThrusterControl started" );
    running = true;
    setHealthToOk();
    setEnabled(true);
    emit started(this);
    running = true;
    if(!this->thrustercontrolloop->isEnabled()){
        this->thrustercontrolloop->setEnabled(true);
    }

    float tempAng = this->getSettingsValue("angularSpeed").toFloat();
    float tempFwd = this->getSettingsValue("forwardSpeed").toFloat();
    float tempDd = this->getSettingsValue("desiredDepth").toFloat();

    if(tempAng > 1.0){
        tempAng = 1.0;
        emit newMessage("TaskThrusterControl: Angular speed set to 1.0");
    } else if(tempAng <-1.0){
        tempAng = -1.0;
        emit newMessage("TaskThrusterControl: Angular speed set to -1.0");
    }

    if(tempFwd > 1.0){
        tempFwd = 1.0;
        emit newMessage("TaskThrusterControl: Forward speed set to 1.0");
    } else if(tempFwd <-1.0){
        tempFwd = -1.0;
        emit newMessage("TaskThrusterControl: Forward speed set to -1.0");
    }

    float tempFinDepth = this->getSettingsValue("commandCenterSubExecuteDepth").toFloat();
    if(tempFinDepth != 0.0){
        if(tempDd < tempFinDepth) {
            tempDd = tempFinDepth;
            emit newMessage("TaskThrusterControl: Cannot set depth lower than submerged execution depth. Desired depth set to " + this->getSettingsValue("commandCenterSubExecuteDepth").toString());
        }
    } else {
        if(tempDd > 5.0){
            tempDd = 5.0;
            emit newMessage("TaskThrusterControl: Desired depth set to 5.0");
        } else if(tempAng <0.0){
            tempAng = 0.0;
            emit newMessage("TaskThrusterControl: Desired depth set to 0.0");
        }
    }
    addData("taskDuration", this->getSettingsValue("taskDuration"));
    addData("desiredDepth",tempDd);
    addData("forwardSpeed", tempFwd);
    addData("angularSpeed", tempAng);
    qDebug("Ma gucken");
    qDebug()<<tempDd;
    emit dataChanged(this);
    emit forwardSpeed(tempFwd);
    emit angularSpeed(tempAng);
    emit setDepth(tempDd);

    testTimer.singleShot(this->getSettingsValue("taskDuration").toInt(),this, SLOT(stop()));
}

void TaskThrusterControl::setRunData(int taskNr){
    if(taskNr == 6){
        this->setSettingsValue("taskDuration", this->getSettingsValue("taskDuration6"));
        this->setSettingsValue("desiredDepth", this->getSettingsValue("desiredDepth6"));
        this->setSettingsValue("forwardSpeed", this->getSettingsValue("forwardSpeed6"));
        this->setSettingsValue("angularSpeed", this->getSettingsValue("angularSpeed6"));
    } else if(taskNr == 5){
        this->setSettingsValue("taskDuration", this->getSettingsValue("taskDuration5"));
        this->setSettingsValue("desiredDepth", this->getSettingsValue("desiredDepth5"));
        this->setSettingsValue("forwardSpeed", this->getSettingsValue("forwardSpeed5"));
        this->setSettingsValue("angularSpeed", this->getSettingsValue("angularSpeed5"));
    } else if(taskNr == 4){
        this->setSettingsValue("taskDuration", this->getSettingsValue("taskDuration4"));
        this->setSettingsValue("desiredDepth", this->getSettingsValue("desiredDepth4"));
        this->setSettingsValue("forwardSpeed", this->getSettingsValue("forwardSpeed4"));
        this->setSettingsValue("angularSpeed", this->getSettingsValue("angularSpeed4"));
    } else if(taskNr == 3){
        this->setSettingsValue("taskDuration", this->getSettingsValue("taskDuration3"));
        this->setSettingsValue("desiredDepth", this->getSettingsValue("desiredDepth3"));
        this->setSettingsValue("forwardSpeed", this->getSettingsValue("forwardSpeed3"));
        this->setSettingsValue("angularSpeed", this->getSettingsValue("angularSpeed3"));
    } else if(taskNr == 2){
        this->setSettingsValue("taskDuration", this->getSettingsValue("taskDuration2"));
        this->setSettingsValue("desiredDepth", this->getSettingsValue("desiredDepth2"));
        this->setSettingsValue("forwardSpeed", this->getSettingsValue("forwardSpeed2"));
        this->setSettingsValue("angularSpeed", this->getSettingsValue("angularSpeed2"));
    } else {
        this->setSettingsValue("taskDuration", this->getSettingsValue("taskDuration1"));
        this->setSettingsValue("desiredDepth", this->getSettingsValue("desiredDepth1"));
        this->setSettingsValue("forwardSpeed", this->getSettingsValue("forwardSpeed1"));
        this->setSettingsValue("angularSpeed", this->getSettingsValue("angularSpeed1"));
    }
}

void TaskThrusterControl::stop(){
    running = false;
    logger->info( "Task thrustercontrol stopped" );

    if (this->isActive())
    {
        this->testTimer.stop();
        emit forwardSpeed(0.0);
        emit angularSpeed(0.0);
        qDebug()<<this->getSettingsValue("commandCenterSubExecuteDepth").toString();
        emit setDepth(this->getSettingsValue("commandCenterSubExecuteDepth").toFloat());
        emit newMessage("TaskThrusterControl: Set desired depth back to " + this->getSettingsValue("commandCenterSubExecuteDepth").toString());

        addData("desiredDepth",this->getSettingsValue("commandCenterSubExecuteDepth").toFloat());
        addData("forwardSpeed", 0.0);
        addData("angularSpeed", 0.0);
        emit dataChanged(this);

        this->setEnabled(false);


        emit finished(this,true);
    }
}


void TaskThrusterControl::emergencyStop()
{
    running = false;
    logger->info( "Task thrustercontrol emergency stopped" );

    if (this->isActive())
    {
        this->testTimer.stop();
        emit forwardSpeed(0.0);
        emit angularSpeed(0.0);
        emit setDepth(0.0);
        this->setEnabled(false);
        emit finished(this,false);
    }
}


void TaskThrusterControl::terminate()
{
    this->stop();
    RobotModule::terminate();
}

QWidget* TaskThrusterControl::createView(QWidget *parent)
{
    return new TaskThrusterControlForm(this, parent);
}

QList<RobotModule*> TaskThrusterControl::getDependencies()
{
    QList<RobotModule*> ret;
    //ret.append(tcl);
    ret.append(sim);
    return ret;
}


void TaskThrusterControl::newSchDesSlot(QString taskName,  QString newD){
    emit newSchDesSignal(taskName, newD);
}

void TaskThrusterControl::setDescriptionSlot(){
    emit setDescriptionSignal();
}
