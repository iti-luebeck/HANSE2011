#include "metabehaviour.h"

#include <Module_ThrusterControlLoop/module_thrustercontrolloop.h>
#include <Module_HandControl/module_handcontrol.h>
#include <Framework/modulesgraph.h>
#include <QtGui>
#include "metabehaviourform.h"
#include <Module_PressureSensor/module_pressuresensor.h>
#include <Behaviour_PipeFollowing/behaviour_pipefollowing.h>
#include <Behaviour_TurnOneEighty/behaviour_turnoneeighty.h>
#include <Behaviour_BallFollowing/behaviour_ballfollowing.h>

MetaBehaviour::MetaBehaviour(QString id, ModulesGraph* graph, Module_ThrusterControlLoop* tcl, Module_HandControl* handControl, Module_PressureSensor* pressure, Behaviour_PipeFollowing* pipe, Behaviour_BallFollowing* ball, Behaviour_TurnOneEighty* o80)
    : RobotModule(id)
{
    this->tcl = tcl;
    this->handControl = handControl;
    this->pressure = pressure;
    this->pipe = pipe;
    this->o80 = o80;
    this->ball = ball;
    this->craph = graph;

    setDefaultValue("targetDepth",0.30);
    setDefaultValue("depthErrorVariance",0.05);
    setDefaultValue("timeout",600);
    setDefaultValue("forwardSpeed",0.3);
}

void MetaBehaviour::init()
{
    // find all behaviours
    foreach(RobotModule* m, craph->getModules()) {
        RobotBehaviour* b = dynamic_cast<RobotBehaviour*>(m);
        if (b)
        {
            logger->debug("Modul ID "+b->getId());
            this->behaviours.append(b);
        }

    }
    logger->debug("#Behaviours " +QString::number(this->behaviours.length()));
    // states: dive; pipe; surface
    addData("state","off");

    emit dataChanged(this);

    timeoutTimer.setSingleShot(true);
    connect(&timeoutTimer, SIGNAL(timeout()), this, SLOT(stateTimeout()));

    connect(pressure, SIGNAL(newDepthData(float)), this, SLOT(depthChanged(float)));
    connect(pipe, SIGNAL(finished(RobotBehaviour*,bool)), this, SLOT(finishedPipe(RobotBehaviour*,bool)));
    connect(o80, SIGNAL(finished(RobotBehaviour*,bool)), this, SLOT(finishedTurn(RobotBehaviour*,bool)));
    connect(pressure, SIGNAL(healthStatusChanged(RobotModule*)), this, SLOT(badHealth(RobotModule*)));

    connect(handControl, SIGNAL(emergencyStop()), this, SLOT(emergencyStop()));
    connect(handControl, SIGNAL(startHandControl()), this, SLOT(startHandControl()));
    /* connect to tcl via signal/slot */
    connect(this,SIGNAL(setDepth(float)),tcl,SLOT(setDepth(float)));
    connect(this,SIGNAL(setForwardSpeed(float)),tcl,SLOT(setForwardSpeed(float)));
    connect(this,SIGNAL(setAngularSpeed(float)),tcl,SLOT(setAngularSpeed(float)));
    /* connect stopAllBehaviours Signal to all Behaviours */
    connect(this,SIGNAL(resetTCL()),tcl,SLOT(reset()));
    connect(this,SIGNAL(stopAllBehaviours()),pipe,SLOT(stop()));
    connect(this,SIGNAL(stopAllBehaviours()),o80,SLOT(stop()));
    connect(this,SIGNAL(stopAllBehaviours()),ball,SLOT(stop()));
    connect(this,SIGNAL(stopAllBehaviours()),handControl,SLOT(stop()));
    /* connect all behaviours via signals slots*/
    connect(this,SIGNAL(startPipeFollow()),pipe,SLOT(startBehaviour()));
    connect(this,SIGNAL(stopPipeFollow()),pipe,SLOT(stop()));
    connect(this,SIGNAL(startBallFollow()),ball,SLOT(startBehaviour()));
    connect(this,SIGNAL(stopBallFollow()),ball,SLOT(stop()));
    connect(this,SIGNAL(startTurnO80()),o80,SLOT(startBehaviour()));
    connect(this,SIGNAL(stopTurnO80()),o80,SLOT(stop()));
    connect(this,SIGNAL(startHandCtr()),handControl,SLOT(startBehaviour()));
    connect(this,SIGNAL(stopHandCtr()),handControl,SLOT(stop()));
}

void MetaBehaviour::emergencyStop()
{
    emit stopAllBehaviours();
    emit resetTCL();
    addData("state","off");
    timeoutTimer.stop();
}

QList<RobotModule*> MetaBehaviour::getDependencies()
{
    QList<RobotModule*> ret;
    ret.append(tcl);
    ret.append(handControl);
    ret.append(pressure);
    RobotModule* m;
    foreach(m,behaviours)
        ret.append(m);
    return ret;
}

QWidget* MetaBehaviour::createView(QWidget* parent)
{
    return new MetaBehaviourForm(this, parent);
}

void MetaBehaviour::reset()
{

}

void MetaBehaviour::terminate()
{
    logger->debug("BLUBBLUB");
//    QTimer::singleShot(0,this,SLOT(emergencyStop()));
    this->emergencyStop();
    RobotModule::terminate();
}

void MetaBehaviour::startHandControl()
{
    RobotBehaviour* thisB = dynamic_cast<RobotBehaviour*>(this->handControl);
    logger->info("Try to start HandCtr");
    if(thisB)
    {
        if(!thisB->isRunning())
            thisB->start();
        logger->info("HandCtr ist available");
    }
    emit stopAllBehaviours();
    emit startHandCtr();
    logger->info("Signal emitted");
}

void MetaBehaviour::depthChanged(float depth)
{
    if (getDataValue("state")=="dive" && fabs(tcl->getDepthError())<getSettingsValue("depthErrorVariance").toFloat()) {
        addData("state", "pipe");
        emit dataChanged(this);
        emit startPipeFollow();
        timeoutTimer.stop();
        timeoutTimer.start(getSettingsValue("timeout").toInt()*1000);
    }
    if (getDataValue("state")=="diveSimple" && fabs(tcl->getDepthError())<getSettingsValue("depthErrorVariance").toFloat()) {
        addData("state", "forward");
        emit setForwardSpeed(getSettingsValue("forwardSpeed").toFloat());
//        emit setAngularSpeed(-0.05);
        timeoutTimer.stop();
        timeoutTimer.start(getSettingsValue("timeout").toInt()*1000);
    }
    if (getDataValue("state")=="diveForPipe" && fabs(tcl->getDepthError())<getSettingsValue("depthErrorVariance").toFloat()) {
        addData("state", "pipeFirstPart");
        emit startPipeFollow();
        timeoutTimer.stop();
        timeoutTimer.start(getSettingsValue("timeout").toInt()*1000);
    }
    if (getDataValue("state")=="surface" && fabs(tcl->getDepthError())<getSettingsValue("depthErrorVariance").toFloat()) {
        addData("state", "off");
        emit dataChanged(this);
        timeoutTimer.stop();
    }
    if (depth>3) {
        addData("state","surface");
        emit dataChanged(this);
        timeoutTimer.stop();
        emit setDepth(0);
    }
}

void MetaBehaviour::testPipe()
{
    addData("state", "pipe");
    emit dataChanged(this);
    emit startPipeFollow();
    timeoutTimer.stop();
    timeoutTimer.start(getSettingsValue("timeout").toInt()*1000);
}

void MetaBehaviour::finishedTurn(RobotBehaviour*, bool success) {
    if (getDataValue("state")=="turn") {
        addData("state","pipeSecondPart");
        emit dataChanged(this);
        emit startPipeFollow();
        timeoutTimer.start(getSettingsValue("timeout").toInt()*1000);
    }
}

void MetaBehaviour::stateTimeout()
{
    addData("state","timeoutFail");
    emit dataChanged(this);
    emergencyStop();
    emit setDepth(0);
    emit setForwardSpeed(0);
    emit setAngularSpeed(0);
}

void MetaBehaviour::badHealth(RobotModule *m)
{
    if (m==pressure) {
        addData("state","fail");
        emit dataChanged(this);
        emit stopAllBehaviours();
        emit setDepth(0);
        timeoutTimer.stop();
    }
}


void MetaBehaviour::pipeFollowForward()
{
    emergencyStop();
    addData("state","dive");
    emit dataChanged(this);
    emit setDepth(getSettingsValue("targetDepth").toFloat());
    timeoutTimer.start(getSettingsValue("timeout").toInt()*1000);
}

void MetaBehaviour::simpleForward()
{
    emergencyStop();

    addData("state","diveSimple");
    emit dataChanged(this);
    emit setDepth(getSettingsValue("targetDepth").toFloat());
    timeoutTimer.start(getSettingsValue("timeout").toInt()*1000);
}

void MetaBehaviour::simple180deg()
{
    emergencyStop();
    QTimer::singleShot(0, o80, SLOT(initialHeadingUpdate()));
    msleep(10000);
    addData("state","180simple");
    emit dataChanged(this);
    timeoutTimer.start(getSettingsValue("timeout").toInt()*1000);
    emit startTurnO80();
}


void MetaBehaviour::fullProgram()
{
    emergencyStop();
    addData("reachedVG", false);

    QTimer::singleShot(0, o80, SLOT(initialHeadingUpdate()));

    addData("state","diveForPipe");
    emit dataChanged(this);
    emit setDepth(getSettingsValue("targetDepth").toFloat());
    timeoutTimer.start(getSettingsValue("timeout").toInt()*1000);
}

void MetaBehaviour::finishedPipe(RobotBehaviour*, bool success) {
    if (getDataValue("state")=="pipe") {
        addData("state","surface");
        emit dataChanged(this);
        emit setDepth(0);
        timeoutTimer.stop();
    }
    if (getDataValue("state")=="pipeFirstPart") {
        addData("state","turn");
        addData("reachedVG", true);
        emit dataChanged(this);
        emit startTurnO80();
        timeoutTimer.stop();
        timeoutTimer.start(getSettingsValue("timeout").toInt()*1000);
    }
    if (getDataValue("state")=="pipeSecondPart") {
        addData("state","ball");
        emit dataChanged(this);
        emit setDepth(1.5);
        emit startBallFollow();
        timeoutTimer.stop();
        timeoutTimer.start(5*60*1000);
    }
}
