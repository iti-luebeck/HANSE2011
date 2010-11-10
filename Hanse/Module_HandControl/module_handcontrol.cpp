#include "module_handcontrol.h"
#include <Module_Thruster/module_thruster.h>
#include <Module_ThrusterControlLoop/module_thrustercontrolloop.h>
#include "handcontrol_form.h"
#include "server.h"

Module_HandControl::Module_HandControl(QString id, Module_ThrusterControlLoop *tcl, Module_Thruster *thrusterLeft, Module_Thruster *thrusterRight, Module_Thruster *thrusterDown, Module_Thruster *thrusterDownFront)
    : RobotBehaviour_MT(id)
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

    connect(this,SIGNAL(setAngularSpeed(float)),tcl,SLOT(setAngularSpeed(float)));
    connect(this,SIGNAL(setForwardSpeed(float)),tcl,SLOT(setForwardSpeed(float)));
    connect(this,SIGNAL(setDepth(float)),tcl,SLOT(setDepth(float)));

    connect(this,SIGNAL(setUpDownSpeed(float)),thrusterDown,SLOT(setSpeed(float)));
    connect(this,SIGNAL(setUpDownSpeed(float)),thrusterDownFront,SLOT(setSpeed(float)));
    connect(this,SIGNAL(setRightSpeed(float)),thrusterRight,SLOT(setSpeed(float)));
    connect(this,SIGNAL(setLeftSpeed(float)),thrusterLeft,SLOT(setSpeed(float)));


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
    RobotBehaviour_MT::terminate();
    newMessage(0,0,0);
    server->close();
}

void Module_HandControl::reset()
{
    RobotBehaviour_MT::reset();

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

    moduleMutex.lock();
    
    if (getSettingsValue("receiver").toString()=="thruster") {
        controlLoop->setEnabled(false);

        logger->debug("set thruster direct");
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

        logger->debug("emit signal to tcl");
        emit setAngularSpeed(angularSpeed/divLR);
        emit setForwardSpeed(forwardSpeed/divFw);
//        controlLoop->setAngularSpeed(angularSpeed/divLR);
//        controlLoop->setForwardSpeed(forwardSpeed/divFw);
        float currentSollTiefe = controlLoop->getDepth();
        float dVal = speedUpDown/divUD;
//        controlLoop->setDepth(currentSollTief8e + dVal);
        emit setDepth(currentSollTiefe + dVal);
    }
    
    moduleMutex.unlock();

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
