#include "behaviour_compassfollowing.h"
#include <QtGui>
#include <Behaviour_CompassFollowing/compassfollowingform.h>

Behaviour_CompassFollowing::Behaviour_CompassFollowing(QString id, Module_ThrusterControlLoop *tcl, Module_Compass *compass)
    : RobotBehaviour(id)
{
    this->compass = compass;
    this->tcl = tcl;
    timer = NULL;
    turnTimer = NULL;
    setEnabled(false);
    turning = false;
//    connect(&timer,SIGNAL(timeout()),this,SLOT(controlLoop()));
//    connect(&turnTimer,SIGNAL(timeout()),this,SLOT(turnNinety()));

//    connect(this,SIGNAL(startControlTimer()),&timer,SLOT(start()));
//    connect(this,SIGNAL(startTurnTimer()),&turnTimer,SLOT(start()));

    connect(this,SIGNAL(newAngularSpeed(float)),tcl,SLOT(setAngularSpeed(float)));
    connect(this,SIGNAL(newForwardSpeed(float)),tcl,SLOT(setForwardSpeed(float)));
}

bool Behaviour_CompassFollowing::isActive()
{
    return isEnabled();
}

void Behaviour_CompassFollowing::start()
{
    timer = new QTimer();
    turnTimer = new QTimer();
    connect(timer,SIGNAL(timeout()),this,SLOT(controlLoop()));
    connect(turnTimer,SIGNAL(timeout()),this,SLOT(turnNinety()));
    logger->info("starting Compass Following");
    this->setEnabled(true);
    addData("ctrHeading",compass->getHeading());
    turning = false;
    emit dataChanged(this);
    timer->start(100);
//    turnTimer->start(getSettingsValue("driveTime").toInt());
}

void Behaviour_CompassFollowing::stop()
{
    this->setEnabled(false);
    timer->stop();
    turnTimer->stop();
    setEnabled(false);
    emit newAngularSpeed(0.0);
    emit newForwardSpeed(0.0);
    emit finished(this,true);
}

void Behaviour_CompassFollowing::reset()
{
    if(timer != NULL)
    {
        timer->stop();
        turnTimer->stop();
    }
    emit newAngularSpeed(0.0);
    emit newForwardSpeed(0.0);
    RobotModule::reset();
    if(compass->getHealthStatus().isHealthOk())
    {
        addData("ctrHeading",compass->getHeading());
        emit dataChanged(this);
        turning = false;
        this->setHealthToOk();
        if(timer != NULL)
        {
            timer->start();
            turnTimer->start();
        }
    }
    else
    {
        this->stopOnCompassError();
    }
}

void Behaviour_CompassFollowing::controlLoop()
{
    logger->info("control compass");
    float ctrAngle = getDataValue("ctrHeading").toFloat();
    if(!compass->getHealthStatus().isHealthOk())
    {
        this->stopOnCompassError();
        return;
    }
    float curHeading = compass->getHeading();
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

void Behaviour_CompassFollowing::turnNinety()
{
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

void Behaviour_CompassFollowing::refreshHeading()
{
    if(compass->getHealthStatus().isHealthOk())
        addData("ctrHeading",compass->getHeading());
    else
        this->stopOnCompassError();
}

void Behaviour_CompassFollowing::stopOnCompassError()
{
    this->setEnabled(false);
    timer->stop();
    turnTimer->stop();
    setHealthToSick("compass error");
    setEnabled(false);
    emit newAngularSpeed(0.0);
    emit newForwardSpeed(0.0);
    emit finished(this,false);
}

QList<RobotModule*> Behaviour_CompassFollowing::getDependencies()
{
    QList<RobotModule*> ret;
    ret.append(tcl);
    ret.append(compass);
    return ret;
}

QWidget* Behaviour_CompassFollowing::createView(QWidget* parent)
{
    return new CompassFollowingForm(parent, this);
}

