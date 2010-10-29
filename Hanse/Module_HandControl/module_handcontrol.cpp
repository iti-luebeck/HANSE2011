#include "module_handcontrol.h"
#include <Module_Thruster/module_thruster.h>
#include <Module_ThrusterControlLoop/module_thrustercontrolloop.h>
#include "handcontrol_form.h"
#include "server.h"

Module_HandControl::Module_HandControl(QString id, Module_ThrusterControlLoop *tcl, Module_Thruster *thrusterLeft, Module_Thruster *thrusterRight, Module_Thruster *thrusterDown, Module_Thruster *thrusterDownFront)
    : RobotBehaviour(id)
{
    this->controlLoop = tcl,
    this->thrusterDown = thrusterDown;
    this->thrusterDownFront = thrusterDownFront;
    this->thrusterLeft = thrusterLeft;
    this->thrusterRight = thrusterRight;

    setEnabled(false);
    setDefaultValue("port",1234);
    setDefaultValue("receiver","thruster");
    setDefaultValue("divLR",127);
    setDefaultValue("divFw",127);
    setDefaultValue("divUD",50);

    server = new Server();
    connect(server,SIGNAL(newMessage(int,int,int)), this, SLOT(newMessage(int,int,int)));
    connect(server, SIGNAL(healthProblem(QString)), this, SLOT(serverReportedError(QString)));
    connect(server, SIGNAL(emergencyStop()), this, SLOT(emergencyStopReceived()));
    connect(server, SIGNAL(startHandControl()), this, SLOT(startHandControlReceived()));

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
    RobotBehaviour::terminate();
    newMessage(0,0,0);
    server->close();
}

void Module_HandControl::reset()
{
    RobotBehaviour::reset();

    newMessage(0,0,0);

    server->close();
    server->open(getSettingsValue("port").toInt());
}

void Module_HandControl::emergencyStopReceived()
{
    emit emergencyStop();
}

void Module_HandControl::newMessage(int forwardSpeed, int angularSpeed, int speedUpDown)
{
    if (!getSettingsValue("enabled").toBool())
        return;

    addData("forwardSpeed", forwardSpeed);
    addData("angularSpeed", angularSpeed);
    addData("speedUpDown", speedUpDown);

    sendNewControls();

    // seems to be working..
    setHealthToOk();

    emit dataChanged(this);
}

void Module_HandControl::sendNewControls()
{
    if (!getSettingsValue("enabled").toBool())
        return;

    int forwardSpeed = getDataValue("forwardSpeed").toInt();
    int angularSpeed = getDataValue("angularSpeed").toInt();
    int speedUpDown = getDataValue("speedUpDown").toInt();

    float divFw = getSettingsValue("divFw").toFloat();
    float divLR = getSettingsValue("divLR").toFloat();
    float divUD = getSettingsValue("divUD").toFloat();

    if (getSettingsValue("receiver").toString()=="thruster") {
        controlLoop->setEnabled(false);

        float left = forwardSpeed/divFw+angularSpeed/divLR;
        float right = forwardSpeed/divFw-angularSpeed/divLR;
        float updown = (float)speedUpDown / divUD;
        thrusterDown->setSpeed(updown);
        thrusterDownFront->setSpeed(updown);
        thrusterLeft->setSpeed(left);
        thrusterRight->setSpeed(right);

    } else {
        if (!controlLoop->isEnabled())
            controlLoop->setEnabled(true);

        controlLoop->setAngularSpeed(angularSpeed/divLR);
        controlLoop->setForwardSpeed(forwardSpeed/divFw);
        float currentSollTiefe = controlLoop->getDepth();
        float dVal = speedUpDown/divUD;
        controlLoop->setDepth(currentSollTiefe + dVal);
    }

}

void Module_HandControl::serverReportedError(QString error)
{
    setHealthToSick(error);
}

void Module_HandControl::start()
{
    setEnabled(true);
    emit started(this);
    reset();
}

void Module_HandControl::stop()
{
    newMessage(0,0,0);
    setEnabled(false);
    emit finished(this, true);
}

bool Module_HandControl::isActive()
{
    return isEnabled();
}

void Module_HandControl::startHandControlReceived()
{
    emit startHandControl();
}
