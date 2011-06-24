#include "behaviour_xsensfollowing.h"
#include <QtGui>
#include <Behaviour_XsensFollowing/xsensfollowingform.h>
#include <Framework/Angles.h>

Behaviour_XsensFollowing::Behaviour_XsensFollowing(QString id, Module_ThrusterControlLoop *tcl, Module_XsensMTi *xsens)
    : RobotBehaviour(id)
{
    this->xsens = xsens;
    this->tcl = tcl;
    turnTimer.moveToThread(this);
    timer.moveToThread(this);

    this->setDefaultValue("timer",30);
    this->setDefaultValue("driveTime",10000);
    this->setDefaultValue("ffSpeed",0.5);
    this->setDefaultValue("kp",0.3);
    this->setDefaultValue("delta",10);
}

bool Behaviour_XsensFollowing::isActive()
{
    return active;
}

void Behaviour_XsensFollowing::init()
{
    active = false;
    logger->info("Xsens Following init");
    setEnabled(false);
    connect(this,SIGNAL(newAngularSpeed(float)),tcl,SLOT(setAngularSpeed(float)));
    connect(this,SIGNAL(newForwardSpeed(float)),tcl,SLOT(setForwardSpeed(float)));
    connect(&timer,SIGNAL(timeout()),this,SLOT(controlLoop()));
    connect(&turnTimer,SIGNAL(timeout()),this,SLOT(turnNinety()));
    connect(this, SIGNAL(enabled(bool)), this, SLOT(controlEnabledChanged(bool)));
}

void Behaviour_XsensFollowing::startBehaviour()
{
    if (!isActive()) {
        logger->info("Already active!");
        return;
    }

    active = true;
    if(!isEnabled()){
        setEnabled(true);
    }


    logger->info("Starting Xsens Following");
    targetHeading = xsens->getHeading();

    emit dataChanged(this);

    timer.start(getSettingsValue("timer").toInt());
    turnTimer.start(getSettingsValue("driveTime").toInt());
}

void Behaviour_XsensFollowing::stop()
{
    if(timer.isActive()){
        timer.stop();
    }
    if(turnTimer.isActive()){
        turnTimer.stop();
    }

    if (!isActive()) {
        logger->info("Not active!");
        return;
    }
    active = false;
    logger->info("Xsens follow stop");
    this->setEnabled(false);
    setEnabled(false);
    emit newAngularSpeed(0.0);
    emit newForwardSpeed(0.0);
    logger->info("Stop Xsens Following");
    emit finished(this,true);
}

void Behaviour_XsensFollowing::reset()
{
    if (!isActive()) {
        return;
    }
    logger->info("Xsens follow reset");

    timer.stop();
    turnTimer.stop();
    emit newAngularSpeed(0.0);
    emit newForwardSpeed(0.0);
    RobotModule::reset();
    if(xsens->isEnabled()){
        if(xsens->getHealthStatus().isHealthOk()){ 
            targetHeading = xsens->getHeading();
            emit dataChanged(this);
            this->setHealthToOk();
            timer.start(getSettingsValue("timer").toInt());
            turnTimer.start(getSettingsValue("driveTime").toInt());
        }else{
            this->stopOnXsensError();
        }
    }else{
        this->stop();
    }
}

void Behaviour_XsensFollowing::controlLoop()
{
    if (!isActive()) {
        return;
    }

    if(!xsens->isEnabled() || !xsens->getHealthStatus().isHealthOk())
    {
        this->stopOnXsensError();
        return;
    }

    double currentHeading = xsens->getHeading();
    double diffHeading = Angles::deg2deg(targetHeading - currentHeading);
    double ctrAngleSpeed = 0.0;

    if(fabs(diffHeading) < getSettingsValue("delta").toDouble())
    {
        ctrAngleSpeed = 0.0;
    } else {
        ctrAngleSpeed = getSettingsValue("kp").toFloat()*diffHeading;
    }


    addData("heading current",currentHeading);
    addData("heading target", targetHeading);
    addData("heading difference", diffHeading);
    emit dataChanged(this);
    emit newAngularSpeed(ctrAngleSpeed);
    emit newForwardSpeed(getSettingsValue("ffSpeed").toFloat());
}

void Behaviour_XsensFollowing::turnNinety()
{
    if (!isActive()) {
        this->stop();
        return;
    }

    logger->info("Turn 90");
    if(getSettingsValue("enableTurn").toBool() == true){
        if(getSettingsValue("turnClockwise").toBool() == true){
            targetHeading = Angles::deg2deg(targetHeading+90);
        } else {
            targetHeading = Angles::deg2deg(targetHeading-90);
        }
    }
}

void Behaviour_XsensFollowing::refreshHeading()
{

    if (!isActive()) {
        return;
    }
    if(xsens->isEnabled()){

        this->dataLockerMutex.lock();

        targetHeading = this->xsens->getHeading();
        addData("heading target", targetHeading);
        dataChanged( this );
        this->dataLockerMutex.unlock();

    }
}

void Behaviour_XsensFollowing::stopOnXsensError()
{
    if (!isActive()) {
        return;
    }

    logger->info("Xsens follow stop error");
    active = false;
    setEnabled(false);
    timer.stop();
    turnTimer.stop();
    setHealthToSick("xsens error");
    setEnabled(false);
    emit newAngularSpeed(0.0);
    emit newForwardSpeed(0.0);
    emit finished(this,false);
}

QList<RobotModule*> Behaviour_XsensFollowing::getDependencies()
{
    QList<RobotModule*> ret;
    ret.append(tcl);
    ret.append(xsens);
    return ret;
}

QWidget* Behaviour_XsensFollowing::createView(QWidget* parent)
{
    return new XsensFollowingForm(parent, this);
}

void Behaviour_XsensFollowing::controlEnabledChanged(bool b){
    if(!b && isActive()){
        logger->info("Disable and deactivate XsensFollowing");
        stop();
    } else if(!b && !isActive()){
        logger->info("Still deactivated");
    } else if(b && !isActive()){
        logger->info("Enable and activate XsensFollowing");
        startBehaviour();
    } else {
        logger->info("Still activated");
    }
}
