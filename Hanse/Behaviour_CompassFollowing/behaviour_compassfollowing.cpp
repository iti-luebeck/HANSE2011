#include "behaviour_compassfollowing.h"
#include <QtGui>
#include <Behaviour_CompassFollowing/compassfollowingform.h>

Behaviour_CompassFollowing::Behaviour_CompassFollowing(QString id, Module_ThrusterControlLoop *tcl, Module_Compass *compass)
    : RobotBehaviour_MT(id)
{
    this->compass = compass;
    this->tcl = tcl;
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
   connect(timer,SIGNAL(timeout()),this,SLOT(controlLoop()));
    logger->info("starting Compass Following");
    this->setEnabled(true);
    addData("ctrHeading",compass->getHeading());
    turning = false;
    emit dataChanged(this);
//    emit startControlTimer();
    timer->start(100);
//    turnTimer.start(getSettingsValue("driveTime").toInt());
}

void Behaviour_CompassFollowing::stop()
{
    this->setEnabled(false);
//    timer.stop();
//    turnTimer.stop();
}

void Behaviour_CompassFollowing::reset()
{
//    timer.stop();
//    turnTimer.stop();
    addData("ctrHeading",compass->getHeading());
    emit dataChanged(this);
    turning = false;
//    timer.start();
//    turnTimer.stop();
}

void Behaviour_CompassFollowing::controlLoop()
{
    logger->info("control compass");
    float ctrAngle = getDataValue("ctrHeading").toFloat();
    float curHeading = compass->getHeading();
    float curDelta = fabs(ctrAngle - curHeading);
    float ctrAngleSpeed = 0.0;
    if(curDelta > getDataValue("delta").toFloat())
    {
        ctrAngleSpeed = getDataValue("kp").toFloat() * curHeading / ctrAngle;
    }
    addData("angularSpeed",ctrAngleSpeed);
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

