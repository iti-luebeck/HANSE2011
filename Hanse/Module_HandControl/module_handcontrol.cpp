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


}

void Module_HandControl::init()
{
    server = new Server();
    server->moveToThread(this);

    connect(server,SIGNAL(newMessage(int,int,int)), this, SLOT(newMessage(int,int,int)));
    connect(server, SIGNAL(healthProblem(QString)), this, SLOT(serverReportedError(QString)));
    connect(server, SIGNAL(emergencyStop()), this, SLOT(emergencyStopReceived()));
    connect(server, SIGNAL(startHandControl()), this, SLOT(startHandControlReceived()));

    connect(this,SIGNAL(stopServer()),server,SLOT(close()));
    connect(this,SIGNAL(startServer()),server,SLOT(open()));

    connect(this,SIGNAL(setAngularSpeed(float)),controlLoop,SLOT(setAngularSpeed(float)));
    connect(this,SIGNAL(setForwardSpeed(float)),controlLoop,SLOT(setForwardSpeed(float)));
    connect(this,SIGNAL(setDepth(float)),controlLoop,SLOT(setDepth(float)));

    connect(this,SIGNAL(setUpDownSpeed(float)),thrusterDown,SLOT(setSpeed(float)));
    connect(this,SIGNAL(setUpDownSpeed(float)),thrusterDownFront,SLOT(setSpeed(float)));
    connect(this,SIGNAL(setRightSpeed(float)),thrusterRight,SLOT(setSpeed(float)));
    connect(this,SIGNAL(setLeftSpeed(float)),thrusterLeft,SLOT(setSpeed(float)));

    reset();

}

void Module_HandControl::createServer()
{
//    server = new ServerMT();
//    server->open(getSettingsValue("port").toInt());
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
    newMessage(0,0,0);
//    server->close();
    emit stopServer();
    RobotModule::terminate();
}

void Module_HandControl::reset()
{
    qDebug() << "hctrl res THREAD ID";
    qDebug() << QThread::currentThreadId();

    RobotModule::reset();

    newMessage(0,0,0);
//server->close();
    emit stopServer();
    msleep(100);

//    server->setPort(getSettingsValue("port").toInt());
    emit startServer();
//    server->open();
//    server->open(getSettingsValue("port").toInt());
}

void Module_HandControl::emergencyStopReceived()
{
    logger->debug("emergency handctr");

     if (getSettingsValue("receiver").toString()=="thruster")
        emit setUpDownSpeed(100.0);
     else
         emit setDepth(0.0);
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

    dataLockerMutex.lock();
    
    if (getSettingsValue("receiver").toString()=="thruster") {
        controlLoop->setEnabled(false);

        logger->debug("set thruster direct");
        float left = forwardSpeed/divFw+angularSpeed/divLR;
        float right = forwardSpeed/divFw-angularSpeed/divLR;
        float updown = (float)speedUpDown / divUD;

        emit setUpDownSpeed(updown);
        emit setRightSpeed(right);
        emit setLeftSpeed(left);
//        thrusterDown->setSpeed(updown);
//        thrusterDownFront->setSpeed(updown);
//        thrusterLeft->setSpeed(left);
//        thrusterRight->setSpeed(right);

    } else {
        if (!controlLoop->isEnabled())
            controlLoop->setEnabled(true);


        emit setAngularSpeed(angularSpeed/divLR);
        emit setForwardSpeed(forwardSpeed/divFw);
//        controlLoop->setAngularSpeed(angularSpeed/divLR);
//        controlLoop->setForwardSpeed(forwardSpeed/divFw);
//        float currentSollTiefe = controlLoop->getDepth();
//        float dVal = speedUpDown/divUD;
        float dVal = 0.0;
        if(speedUpDown > 0)
            dVal = divUD;
        else if(speedUpDown < 0)
            dVal = -divUD;
//        controlLoop->setDepth(currentSollTief8e + dVal);
        float depth = dVal + controlLoop->getDepth();
        logger->debug("emit signal to tcl containing depth: "+QString::number(depth));
        emit setDepth(depth);
    }
    
    dataLockerMutex.unlock();

}

void Module_HandControl::serverReportedError(QString error)
{
    setHealthToSick(error);
}

void Module_HandControl::startBehaviour()
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
    logger->debug("start handctr");
    emit startHandControl();
}
