#include <Module_ThrusterControlLoop/module_thrustercontrolloop.h>
#include <Module_Thruster/module_thruster.h>
#include <Module_PressureSensor/module_pressuresensor.h>
#include "tcl_form.h"
#include <Framework/healthstatus.h>

Module_ThrusterControlLoop::Module_ThrusterControlLoop(QString id, Module_PressureSensor *pressure, Module_Thruster *thrusterLeft, Module_Thruster *thrusterRight, Module_Thruster *thrusterDown, Module_Thruster* thrusterDownFront)
    : RobotModule(id)
{
    this->thrusterDown = thrusterDown;
    this->thrusterLeft = thrusterLeft;
    this->thrusterRight = thrusterRight;
    this->thrusterDownFront = thrusterDownFront;
    this->pressure = pressure;


    dataLockerMutex.lock();
    pressureSensor_isHealthOK=true;
    dataLockerMutex.unlock();

    setDefaultValue("Kp", 0.5);
    setDefaultValue("Ti", 2.0);
    setDefaultValue("Td", 0.0);
    setDefaultValue("neutralSpeed", 0.0);
    setDefaultValue("minSpeed", -0.3);
    setDefaultValue("maxSpeed", 0.3);
    setDefaultValue("neutralSpeed", 0.0);
    setDefaultValue("minHysteresis", -1.0);
    setDefaultValue("maxHysteresis", 0.2);
    setDefaultValue("use pid", true);

    setDefaultValue("horizSpM_exp", false);
    setDefaultValue("ignoreHealth", false);

    pidController = new PIDController();
    pidController->setValues(Kp, Ti, Td, neutralSpeed, minSpeed, maxSpeed);
}

void Module_ThrusterControlLoop::terminate()
{
    reset();
    RobotModule::terminate();
}

void Module_ThrusterControlLoop::init()
{
    updateConstantsFromInitNow();

    reset();

    connect(pressure, SIGNAL(newDepthData(float)), this, SLOT(newDepthData(float)));
    connect(pressure, SIGNAL(healthStatusChanged(HealthStatus)), this, SLOT(healthStatusChanged(HealthStatus)));
    //healthStatusChanged
    connect(this,SIGNAL(setLeftThruster(float)),thrusterLeft,SLOT(setSpeed(float)));
    connect(this,SIGNAL(setRightThruster(float)),thrusterRight,SLOT(setSpeed(float)));
    connect(this,SIGNAL(setUpDownThrusterFront(float)),thrusterDownFront,SLOT(setSpeed(float)));
    connect(this,SIGNAL(setUpDownThrusterBack(float)),thrusterDown,SLOT(setSpeed(float)));

}

void Module_ThrusterControlLoop::reset()
{
    QMutexLocker l(&dataLockerMutex);
    RobotModule::reset();
    actualForwardSpeed=0.0;
    actualAngularSpeed=0.0;
    setvalueDepth=0.0;
    paused = false;

    control_loop_enabled = false;

    emit setLeftThruster(0);
    emit setRightThruster(0);
    emit setUpDownThrusterFront(0);
    emit setUpDownThrusterBack(0);
}

void Module_ThrusterControlLoop::updateConstantsFromInitNow()
{
    QMutexLocker l(&dataLockerMutex);
    Kp = getSettingsValue("Kp").toFloat();
    Ti = getSettingsValue("Ti").toFloat();
    Td = getSettingsValue("Td").toFloat();
    neutralSpeed = getSettingsValue("neutralSpeed").toFloat();
    minSpeed = getSettingsValue("minSpeed").toFloat();
    maxSpeed = getSettingsValue("maxSpeed").toFloat();
    minHysteresis = getSettingsValue("minHysteresis").toFloat();
    maxHysteresis = getSettingsValue("maxHysteresis").toFloat();

    horizSpM_exp = getSettingsValue("horizSpM_exp").toBool();
    ignoreHealth = getSettingsValue("ignoreHealth").toBool();

    pidController->setValues(Kp, Ti, Td, neutralSpeed, minSpeed, maxSpeed);
}

void Module_ThrusterControlLoop::healthStatusChanged(HealthStatus pressureSensorHealth) {
    QMutexLocker l(&dataLockerMutex);
    if (!getSettingsValue("enabled").toBool())
        return;

    this->pressureSensor_isHealthOK = pressureSensorHealth.isHealthOk();

}

void Module_ThrusterControlLoop::newDepthData(float depth)
{
    QMutexLocker l(&dataLockerMutex);
    if (!getSettingsValue("enabled").toBool())
        return;

    addData("depth actual", depth);

    if (control_loop_enabled) {
        // Speed of the UpDownThruster:
        // TODO: PRESUMPTION: speed>0.0 means UP

        float speed = 0;

        if (getSettingsValue("use pid").toBool()) {
            bool ok = true;
            speed = pidController->nextControlValue(setvalueDepth, depth, ok);
            if (ok) {
                setHealthToOk();
            } else {
                setHealthToSick("zu lange kein neuer Wert");
            }
            emit healthStatusChanged(this);
        } else {
            speed = Kp * (setvalueDepth - depth);
            if (speed < minSpeed) speed = minSpeed;
            if (speed > maxSpeed) speed = maxSpeed;
        }
        addData("depth speed", speed);

        //// Health-Check ////
        // Can we believe the pressure sensor?
        // If not stop the thrusterDown!
        if (!pressureSensor_isHealthOK) { speed = neutralSpeed; }

        if (abs(setvalueDepth - depth)>getSettingsValue("forceUnpauseError").toFloat())
            paused = false;

        if (paused)
            return;

        emit setUpDownThrusterFront(speed);
        emit setUpDownThrusterBack(speed);

        emit dataChanged(this);
    }
}

void Module_ThrusterControlLoop::updateHorizontalThrustersNow()
{
    QMutexLocker l(&dataLockerMutex);
    float speedL=0.0;
    float speedR=0.0;
#define MINSP 0.01

    if (fabs(actualForwardSpeed)<MINSP) {
        // just turn:
        speedL =  actualAngularSpeed;
        speedR = -actualAngularSpeed;

    } else if (fabs(actualAngularSpeed)<MINSP) {
        // just go forward:
        speedL = actualForwardSpeed;
        speedR = actualForwardSpeed;

    } else {
        // combine 'forward' and 'angular' speed:

        if (!horizSpM_exp) { // the easy way:
            speedL = (actualForwardSpeed+actualAngularSpeed)/2;
            speedR = (actualForwardSpeed-actualAngularSpeed)/2;

        } else { // the experimental way:
            // 1. Just add the 'forward' and 'angular' speed:
            speedL = actualForwardSpeed+actualAngularSpeed;
            speedR = actualForwardSpeed-actualAngularSpeed;

            // 2. If one of the speeds are out of bounds then
            // normalize it to the max-abs-value:
            float aL = fabs(speedL);
            float aR = fabs(speedR);
            if ((aL>1.0) && (aL>aR)) {
                speedL = speedL/aL;
                speedR = speedR/aL;
            } else if (aR>1.0) {
                speedL = speedL/aR;
                speedR = speedR/aR;
            }
        }
    }

    emit setLeftThruster(speedL);
    emit setRightThruster(speedR);
//    thrusterLeft->setSpeed( speedL);
//    thrusterRight->setSpeed(speedR);

}

void Module_ThrusterControlLoop::setAngularSpeed(float angularSpeed)
{
    QMutexLocker l(&dataLockerMutex);
    if (!getSettingsValue("enabled").toBool() || paused)
        return;

    if (angularSpeed> 1.0) { angularSpeed= 1.0; }
    if (angularSpeed<-1.0) { angularSpeed=-1.0; }
    actualAngularSpeed=angularSpeed;    
    addData("speed angular", actualAngularSpeed);

    updateHorizontalThrustersNow();
    emit dataChanged(this);
}

void Module_ThrusterControlLoop::setForwardSpeed(float speed)
{
    QMutexLocker l(&dataLockerMutex);
    //qDebug() << "tcl THREAD ID";
    //qDebug() << QThread::currentThreadId();

    if (!getSettingsValue("enabled").toBool() || paused)
        return;

    if (speed> 1.0) { speed= 1.0; }
    if (speed<-1.0) { speed=-1.0; }
    actualForwardSpeed=speed;
    addData("speed forward", actualForwardSpeed);  // for logging

    updateHorizontalThrustersNow();
    emit dataChanged(this);
}

void Module_ThrusterControlLoop::setDepth(float depth)
{
    QMutexLocker l(&dataLockerMutex);
    if (!getSettingsValue("enabled").toBool() || paused)
        return;

    control_loop_enabled=true;

    if (depth < 0)
        depth = 0;

    if (depth > 5)
        depth = 5;

    addData("depth target", depth);
    emit dataChanged(this);
    setvalueDepth=depth;
}

QWidget* Module_ThrusterControlLoop::createView(QWidget *parent)
{
    return new TCL_Form(this, parent);
}

QList<RobotModule*> Module_ThrusterControlLoop::getDependencies()
{
    QList<RobotModule*> ret;
    ret.append(pressure);
    ret.append(thrusterDown);
    ret.append(thrusterLeft);
    ret.append(thrusterRight);
    return ret;
}

float Module_ThrusterControlLoop::getDepth()
{
    return getDataValue("depthSoll").toFloat();
}

float Module_ThrusterControlLoop::getForwardSpeed()
{
    return getDataValue("actualForwardSpeed").toFloat();
}

float Module_ThrusterControlLoop::getAngularSpeed()
{
    return getDataValue("actualAngularSpeed").toFloat();
}

float Module_ThrusterControlLoop::getDepthError()
{
    return getDataValue("depth_error").toFloat();
}
