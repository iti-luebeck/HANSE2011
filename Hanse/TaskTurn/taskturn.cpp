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
    this->setDefaultValue("forwardSpeed1",0.0);
    this->setDefaultValue("angularSpeed1",0.2);
    this->setDefaultValue("degree1",90);
    this->setDefaultValue("tolerance1",0);
    this->setDefaultValue("update1",100);
    this->setDefaultValue("p1",0.4);
    this->setDefaultValue("description1","task1");

    this->setDefaultValue("forwardSpeed2",0.0);
    this->setDefaultValue("angularSpeed2",0.3);
    this->setDefaultValue("degree2",180);
    this->setDefaultValue("tolerance2",10);
    this->setDefaultValue("update2",100);
    this->setDefaultValue("p2",0.4);
    this->setDefaultValue("description2","task2");

    this->setDefaultValue("forwardSpeed3",0.0);
    this->setDefaultValue("angularSpeed3",0.4);
    this->setDefaultValue("degree3",360);
    this->setDefaultValue("tolerance3",15);
    this->setDefaultValue("update3",200);
    this->setDefaultValue("p3",0.4);
    this->setDefaultValue("description3","task3");

    connect(this, SIGNAL(end()), this, SLOT(stop()));

    controlTimer = new QTimer(this);
    connect(controlTimer, SIGNAL(timeout()), this, SLOT(controlTurn()));

    connect(this,SIGNAL(forwardSpeed(float)),thrustercontrolloop,SLOT(setForwardSpeed(float)));
    connect(this,SIGNAL(angularSpeed(float)),thrustercontrolloop,SLOT(setAngularSpeed(float)));
    connect(this,SIGNAL(setDepth(float)),thrustercontrolloop,SLOT(setDepth(float)));

    currentHeading = 0.0;
    targetHeading = 0.0;
    tolerance = 0;
    angSpeed = 0.0;
    fwdSpeed = 0.0;
    initialHeading = 0.0;
    diffHeading = 0.0;

}

bool TaskTurn::isActive(){
    return isEnabled();
}


void TaskTurn::init(){
    logger->debug("taskturn init");
}



void TaskTurn::startBehaviour(){

    emit angularSpeed(0.0);
    emit forwardSpeed(0.0);
    this->reset();
    logger->info("Taskturn started" );
    running = true;
    setHealthToOk();
    setEnabled(true);
    emit started(this);
    running = true;
    addData("degree", this->getSettingsValue("degree"));
    addData("tolerance", this->getSettingsValue("tolerance"));
    addData("update", this->getSettingsValue("update"));
    addData("forwardSpeed", this->getSettingsValue("forwardSpeed"));
    addData("angularSpeed", this->getSettingsValue("angularSpeed"));
    addData("p", this->getSettingsValue("p"));


    initialHeading = this->xsens->getHeading();
    targetHeading = initialHeading +this->getSettingsValue("degree").toFloat();
    addData("initialHeading", initialHeading);
    addData("targetHeading", targetHeading);
    emit dataChanged(this);
    controlTimer->start(this->getSettingsValue("update").toInt());
}

void TaskTurn::setRunData(int taskNr){
    if(taskNr == 3){
        this->setSettingsValue("degree", this->getSettingsValue("degree3"));
        this->setSettingsValue("tolerance", this->getSettingsValue("tolerance3"));
        this->setSettingsValue("forwardSpeed", this->getSettingsValue("forwardSpeed3"));
        this->setSettingsValue("angularSpeed", this->getSettingsValue("angularSpeed3"));
        this->setSettingsValue("update", this->getSettingsValue("update3"));
        this->setSettingsValue("p", this->getSettingsValue("p3"));
    } else if (taskNr == 2){
        this->setSettingsValue("degree", this->getSettingsValue("degree2"));
        this->setSettingsValue("tolerance", this->getSettingsValue("tolerance2"));
        this->setSettingsValue("forwardSpeed", this->getSettingsValue("forwardSpeed2"));
        this->setSettingsValue("angularSpeed", this->getSettingsValue("angularSpeed2"));
        this->setSettingsValue("update", this->getSettingsValue("update2"));
        this->setSettingsValue("p", this->getSettingsValue("p2"));
    } else {
        this->setSettingsValue("degree", this->getSettingsValue("degree1"));
        this->setSettingsValue("tolerance", this->getSettingsValue("tolerance1"));
        this->setSettingsValue("forwardSpeed", this->getSettingsValue("forwardSpeed1"));
        this->setSettingsValue("angularSpeed", this->getSettingsValue("angularSpeed1"));
        this->setSettingsValue("update", this->getSettingsValue("update1"));
        this->setSettingsValue("p", this->getSettingsValue("p1"));
    }
}

void TaskTurn::controlTurn(){
    qDebug("control Turn");
    currentHeading = this->xsens->getHeading();

    qDebug("Current heading:");
    qDebug()<<currentHeading;

    if ( targetHeading > 360 ){
        targetHeading = targetHeading-360;
    }

    diffHeading = targetHeading - currentHeading;
    if ( diffHeading > 180 ){
        diffHeading = diffHeading-360;
    }
    if ( diffHeading < -180 ){
        diffHeading = diffHeading+360;
    }

    addData("currentHeading", currentHeading);
    addData("diffHeading", diffHeading);
    emit dataChanged(this);

    if(diffHeading < tolerance){
        emit end();
    } else {
        diffHeading /= 180;
        float calcAngular = this->getSettingsValue("p").toFloat()*diffHeading;

        emit angularSpeed(calcAngular);

        addData("angSpeed", calcAngular);
        emit dataChanged(this);
        emit forwardSpeed(this->getSettingsValue("forwardSpeed").toFloat());
        //emit setDepth();
    }

}




void TaskTurn::stop(){
    running = false;
    logger->info( "Task turn stopped" );

    if (this->isActive())
    {
        controlTimer->stop();
        this->setEnabled(false);
        emit finished(this,true);
    }
}


void TaskTurn::emergencyStop()
{
    running = false;
    logger->info( "Task turn emergency stopped" );

    if (this->isActive())
    {
        controlTimer->stop();
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
