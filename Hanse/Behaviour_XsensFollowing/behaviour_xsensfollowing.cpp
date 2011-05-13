#include "behaviour_xsensfollowing.h"
#include <QtGui>
#include <Behaviour_XsensFollowing/xsensfollowingform.h>

Behaviour_XsensFollowing::Behaviour_XsensFollowing(QString id, Module_ThrusterControlLoop *tcl, Module_XsensMTi *xsens)
    : RobotBehaviour(id)
{
    this->xsens = xsens;
    this->tcl = tcl;
    turnTimer.moveToThread(this);
    timer.moveToThread(this);

    this->setDefaultValue("timer",100);
    this->setDefaultValue("driveTime",30);
    this->setDefaultValue("ffSpeed",0.3);
    this->setDefaultValue("kp",0.3);
    this->setDefaultValue("delta",0.3);
}

bool Behaviour_XsensFollowing::isActive()
{
    return isEnabled();
}

void Behaviour_XsensFollowing::init()
{
    logger->info("Xsens Following init");
    setEnabled(false);
    turning = false;
    connect(this,SIGNAL(newAngularSpeed(float)),tcl,SLOT(setAngularSpeed(float)));
    connect(this,SIGNAL(newForwardSpeed(float)),tcl,SLOT(setForwardSpeed(float)));
    connect(&timer,SIGNAL(timeout()),this,SLOT(controlLoop()));
    connect(&turnTimer,SIGNAL(timeout()),this,SLOT(turnNinety()));
}

void Behaviour_XsensFollowing::startBehaviour()
{
    logger->info("Starting Xsens Following");
    this->setEnabled(true);
    addData("ctrHeading",xsens->getHeading());
    turning = false;
    emit dataChanged(this);

    timer.start(getSettingsValue("timer").toInt());
    turnTimer.start(getSettingsValue("driveTime").toInt());
}

void Behaviour_XsensFollowing::stop()
{
    logger->info("Xsens follow stop");
    this->setEnabled(false);
    timer.stop();
    turnTimer.stop();
    setEnabled(false);
    emit newAngularSpeed(0.0);
    emit newForwardSpeed(0.0);
    logger->info("Stop Xsens Following");
    emit finished(this,true);
}

void Behaviour_XsensFollowing::reset()
{
    logger->info("Xsens follow reset");
    timer.stop();
    turnTimer.stop();
    emit newAngularSpeed(0.0);
    emit newForwardSpeed(0.0);
    RobotModule::reset();
    if(xsens->getHealthStatus().isHealthOk())
    {
        addData("ctrHeading",xsens->getHeading());
        emit dataChanged(this);
        turning = false;
        this->setHealthToOk();
        timer.start(getSettingsValue("timer").toInt());
        turnTimer.start(getSettingsValue("driveTime").toInt());
    }
    else
    {
        this->stopOnXsensError();
    }
}

void Behaviour_XsensFollowing::controlLoop()
{
    logger->debug("Xsens follow controlloop");
    if (!isActive())
        return;

    float ctrAngle = getDataValue("ctrHeading").toFloat();
    if(!xsens->getHealthStatus().isHealthOk())
    {
        this->stopOnXsensError();
        return;
    }
    float curHeading = xsens->getHeading();
    float curDelta = fabs(ctrAngle - curHeading);
    float ctrAngleSpeed = 0.0;
    float faktor = 1.0;
    if(ctrAngle-curHeading < 0)
        faktor = -1.0;
    if(curDelta > getSettingsValue("delta").toFloat())
    {
        ctrAngleSpeed = getSettingsValue("kp").toFloat()* faktor * curHeading / ctrAngle;
    }
    addData("angularSpeed",ctrAngleSpeed);
    addData("current HEading",curHeading);
    emit dataChanged(this);
    emit newAngularSpeed(ctrAngleSpeed);
    emit newForwardSpeed(getSettingsValue("ffSpeed").toFloat());
}

void Behaviour_XsensFollowing::turnNinety()
{
    logger->debug("Xsens follow turnninety");
    if (!isActive())
        return;

    float ctrAngle = getDataValue("ctrHeading").toFloat();
    float newHeading = ctrAngle;
    if(getSettingsValue("turnClockwise").toBool())
    {
        newHeading = newHeading+90.0;
        if(newHeading > 360)
            newHeading = newHeading - 360;
    }
    else
    {
        newHeading = newHeading-90;
        if(newHeading < 0)
            newHeading =360-newHeading;
    }
    addData("ctrHeading",newHeading);
}

void Behaviour_XsensFollowing::refreshHeading()
{
    logger->debug("Xsens follow refresh heading");
    if (!isActive())
        return;

    if(xsens->getHealthStatus().isHealthOk())
        addData("ctrHeading",xsens->getHeading());
    else
        this->stopOnXsensError();
}

void Behaviour_XsensFollowing::stopOnXsensError()
{
    logger->info("Xsens follow stop error");
    this->setEnabled(false);
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

