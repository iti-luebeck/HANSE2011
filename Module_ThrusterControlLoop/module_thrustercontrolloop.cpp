#include "module_thrustercontrolloop.h"
#include "tcl_form.h"

Module_ThrusterControlLoop::Module_ThrusterControlLoop(QString id, Module_PressureSensor *pressure, Module_Thruster *thrusterLeft, Module_Thruster *thrusterRight, Module_Thruster *thrusterDown)
    : RobotModule(id)
{
    this->thrusterDown = thrusterDown;
    this->thrusterLeft = thrusterLeft;
    this->thrusterRight = thrusterRight;
    this->pressure = pressure;

    setDefaultValue("p", 5);
    setDefaultValue("i", 0);
    setDefaultValue("d", 42);
    setDefaultValue("gap", 1);

    connect(pressure, SIGNAL(newDepthData(float)), this, SLOT(newDepthData(float)));
}

void Module_ThrusterControlLoop::terminate()
{
    // TODO
}

void Module_ThrusterControlLoop::reset()
{
    // TODO
}

void Module_ThrusterControlLoop::newDepthData(float depth)
{
    float p = getSettings().value("p").toFloat();
    float i = getSettings().value("i").toFloat();
    float d = getSettings().value("d").toFloat();

    thrusterLeft->setSpeed(0.5);
    thrusterRight->setSpeed(0.5);
    thrusterDown->setSpeed(0);

    // TODO
}

void Module_ThrusterControlLoop::setAngularSpeed(float angularSpeed)
{
    // TODO
}

void Module_ThrusterControlLoop::setForwardSpeed(float speed)
{
    // TODO
}

void Module_ThrusterControlLoop::setDepth(float depth)
{
    // TODO
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
