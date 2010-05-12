#include "module_handcontrol.h"
#include <Module_Thruster/module_thruster.h>
#include <Module_ThrusterControlLoop/module_thrustercontrolloop.h>
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
    setDefaultValue("divLR",127);
    setDefaultValue("divFw",127);
    setDefaultValue("divUD",50);

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
    RobotModule::terminate();
    server->close();
}

void Module_HandControl::reset()
{
    RobotModule::reset();

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

    float divFw = settings.value("divFw").toFloat();
    float divLR = settings.value("divLR").toFloat();
    float divUD = settings.value("divUD").toFloat();

    if (settings.value("receiver").toString()=="thruster") {
        float left = forwardSpeed/divFw+angularSpeed/divLR;
        float right = forwardSpeed/divFw-angularSpeed/divLR;
        float updown = (float)speedUpDown / divUD;
        thrusterDown->setSpeed(updown);
        thrusterLeft->setSpeed(left);
        thrusterRight->setSpeed(right);

    } else {
        controlLoop->setAngularSpeed(angularSpeed/divLR);
        controlLoop->setForwardSpeed(forwardSpeed/divFw);
        float dVal = speedUpDown/divUD;
        if (dVal<0)
            dVal=0;
        controlLoop->setDepth(dVal);
    }

    // seems to be working..
    setHealthToOk();

    emit dataChanged(this);
}

void Module_HandControl::serverReportedError(QString error)
{
    setHealthToSick(error);
}
