#include "module_thrustercontrolloop.h"
#include "module_thruster.h"
#include "module_pressuresensor.h"
#include "tcl_form.h"

Module_ThrusterControlLoop::Module_ThrusterControlLoop(QString id, Module_PressureSensor *pressure, Module_Thruster *thrusterLeft, Module_Thruster *thrusterRight, Module_Thruster *thrusterDown)
    : RobotModule(id)
{
    this->thrusterDown = thrusterDown;
    this->thrusterLeft = thrusterLeft;
    this->thrusterRight = thrusterRight;
    this->pressure = pressure;

    setDefaultValue("p_up",     1.0);
    setDefaultValue("p_down",   1.0);
    setDefaultValue("maxSpU",   0.3);
    setDefaultValue("maxSpD",   -0.3);
    setDefaultValue("neutrSpD", 0.0);
    setDefaultValue("maxDepthError", 0.05);

    setDefaultValue("horizSpM_exp", false);

    updateConstantsFromInitNow();

    reset();

    connect(pressure, SIGNAL(newDepthData(float)), this, SLOT(newDepthData(float)));
}

void Module_ThrusterControlLoop::terminate()
{
    RobotModule::terminate();
    reset();
}

void Module_ThrusterControlLoop::reset()
{
    actualForwardSpeed=0.0;
    actualAngularSpeed=0.0;
    setvalueDepth=0.0;

    control_loop_enabled = false;

    thrusterLeft->setSpeed( 0 );
    thrusterRight->setSpeed(0 );
    thrusterDown->setSpeed( 0 );
}

void Module_ThrusterControlLoop::updateConstantsFromInitNow()
{
    p_down    = getSettings().value("p_down").toFloat();
    p_up      = getSettings().value("p_up").toFloat();
    maxSpD    = getSettings().value("maxSpD").toFloat();
    maxSpU    = getSettings().value("maxSpU").toFloat();
    neutrSpD  = getSettings().value("neutrSpD").toFloat();
    maxDepthError = getSettings().value("maxDepthError").toFloat();

    horizSpM_exp = getSettings().value("horizSpM_exp").toBool();
}



void Module_ThrusterControlLoop::newDepthData(float depth)
{
    if (!getSettings().value("enabled").toBool())
        return;

    if (control_loop_enabled) {

        // Speed of the UpDownThruster:
        // TODO: PRESUMPTION: speed>0.0 means UP
        float speed = neutrSpD;
        float error = depth-setvalueDepth;

        data["depth_error"] = error; // for logging

        // control-loop step:
        if (fabs(error) > maxDepthError) {
            if (error > 0.0) {
                // We are to deep => go UP:
                speed += p_up*error;
            } else {
                // We are not deep enough
                speed += p_down*error;
            }
        }

        // limit the speed:
        if (speed>maxSpU) { speed=maxSpU; }
        if (speed<maxSpD) { speed=maxSpD; }

        thrusterDown->setSpeed(speed);
        emit dataChanged(this);
    }
}

void Module_ThrusterControlLoop::updateHorizontalThrustersNow()
{
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


    thrusterLeft->setSpeed( speedL);
    thrusterRight->setSpeed(speedR);

}

void Module_ThrusterControlLoop::setAngularSpeed(float angularSpeed)
{
    if (!getSettings().value("enabled").toBool())
        return;

    if (angularSpeed> 1.0) { angularSpeed= 1.0; }
    if (angularSpeed<-1.0) { angularSpeed=-1.0; }
    actualAngularSpeed=angularSpeed;    
    data["actualAngularSpeed"] = actualAngularSpeed;

    updateHorizontalThrustersNow();
    emit dataChanged(this);
}

void Module_ThrusterControlLoop::setForwardSpeed(float speed)
{
    if (!getSettings().value("enabled").toBool())
        return;

    if (speed> 1.0) { speed= 1.0; }
    if (speed<-1.0) { speed=-1.0; }
    actualForwardSpeed=speed;
    data["actualForwardSpeed"] = actualForwardSpeed;  // for logging

    updateHorizontalThrustersNow();
    emit dataChanged(this);
}

void Module_ThrusterControlLoop::setDepth(float depth)
{
    if (!getSettings().value("enabled").toBool())
        return;

    control_loop_enabled=true;

    data["depthSoll"] = depth;

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
