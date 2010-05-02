#include "module_handcontrol.h"
#include "module_thruster.h"
#include "module_thrustercontrolloop.h"
#include "handcontrol_form.h"
#include "server.h"

Module_HandControl::Module_HandControl(QString id, Module_ThrusterControlLoop *tcl, Module_Thruster *thrusterLeft, Module_Thruster *thrusterRight, Module_Thruster *thrusterDown)
    : RobotModule(id)
{
    this->controlLoop = tcl,
    this->thrusterDown = thrusterDown;
    this->thrusterLeft = thrusterLeft;
    this->thrusterRight = thrusterRight;

    setDefaultValue("port",1234);
    setDefaultValue("receiver","thruster");
    setDefaultValue("divisor",127);

    server = new Server();
    connect(server,SIGNAL(newMessage(int,int,int)), this, SLOT(newMessage(int,int,int)));
    connect(server, SIGNAL(healthProblem(QString)), this, SLOT(serverReportedError(QString)));

    reset();
}

QList<RobotModule*> Module_HandControl::getDependencies()
{
    QList<RobotModule*> ret;
    ret.append(thrusterDown);
    ret.append(thrusterLeft);
    ret.append(thrusterRight);
    ret.append(controlLoop);
    return ret;
}

QWidget* Module_HandControl::createView(QWidget* parent)
{
    return new HandControl_Form(this, parent);
}

void Module_HandControl::terminate()
{
    server->close();
}

void Module_HandControl::reset()
{
    server->close();
    server->open(settings.value("port").toInt());
}

void Module_HandControl::newMessage(int forwardSpeed, int angularSpeed, int speedUpDown)
{
    if (!getSettings().value("enabled").toBool())
        return;

    data["forwardSpeed"] = forwardSpeed;
    data["angularSpeed"] = angularSpeed;
    data["speedUpDown"] = speedUpDown;

    float div = settings.value("divisor").toFloat();

    if (settings.value("receiver").toString()=="thruster") {
        float left = (float)(forwardSpeed-angularSpeed) / div;
        float right = (float)(forwardSpeed+angularSpeed) / div;
        float updown = (float)speedUpDown / div;
        thrusterDown->setSpeed(updown);
        thrusterLeft->setSpeed(left);
        thrusterRight->setSpeed(right);

    } else {
        // TODO: find sutable factors once the control loop interface is fixed.
        controlLoop->setAngularSpeed(angularSpeed/div);
        controlLoop->setForwardSpeed(forwardSpeed/div);
        controlLoop->setDepth(speedUpDown/div);
    }

    // seems to be working..
    setHealthToOk();
}

void Module_HandControl::serverReportedError(QString error)
{
    setHealthToSick(error);
}
