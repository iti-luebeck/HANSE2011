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
    : RobotModule_MT(id)
{
    this->tcl = tcl;
    this->handControl = handControl;
    this->pressure = pressure;
    this->pipe = pipe;
    this->o80 = o80;
    this->ball = ball;

    // find all behaviours
    foreach(RobotModule* m, graph->getModules()) {
        RobotBehaviour* b = dynamic_cast<RobotBehaviour*>(m);
        if (b)
        {
            logger->debug("Modul ID "+b->getId());
            this->behaviours.append(b);
        }
        RobotBehaviour_MT* bmt = dynamic_cast<RobotBehaviour_MT*>(m);
        if(bmt)
        {
            logger->debug("ModulMT ID "+bmt->getId());
            this->behavioursMT.append(bmt);
        }

    }

    logger->debug("#Behaviours " +QString::number(this->behaviours.length()));
    logger->debug("#BehavioursMT " +QString::number(this->behavioursMT.length()));


    // states: dive; pipe; surface
    addData("state","off");

    emit dataChanged(this);

    timeoutTimer.setSingleShot(true);
    connect(&timeoutTimer, SIGNAL(timeout()), this, SLOT(stateTimeout()));

    connect(pressure, SIGNAL(newDepthData(float)), this, SLOT(depthChanged(float)));
    connect(pipe, SIGNAL(finished(RobotBehaviour_MT*,bool)), this, SLOT(finishedPipe(RobotBehaviour_MT*,bool)));
    connect(o80, SIGNAL(finished(RobotBehaviour_MT*,bool)), this, SLOT(finishedTurn(RobotBehaviour_MT*,bool)));
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
    connect(this,SIGNAL(startPipeFollow()),pipe,SLOT(start()));
    connect(this,SIGNAL(stopPipeFollow()),pipe,SLOT(stop()));
    connect(this,SIGNAL(startBallFollow()),ball,SLOT(start()));
    connect(this,SIGNAL(stopBallFollow()),ball,SLOT(stop()));
    connect(this,SIGNAL(startTurnO80()),o80,SLOT(start()));
    connect(this,SIGNAL(stopTurnO80()),o80,SLOT(stop()));
    connect(this,SIGNAL(startHandCtr()),handControl,SLOT(start()));
    connect(this,SIGNAL(stopHandCtr()),handControl,SLOT(stop()));
}

void MetaBehaviour::emergencyStop()
{
//    foreach (RobotBehaviour* b, behaviours) {
//        b->stop();
//    }
    emit stopAllBehaviours();
    emit resetTCL();
//    tcl->reset();

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
    foreach(m,behavioursMT)
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
    RobotModule_MT::terminate();
}

void MetaBehaviour::startHandControl()
{
//    RobotBehaviour* thisB = dynamic_cast<RobotBehaviour*>(this->handControl);
//    RobotBehaviour_MT* thisB = dynamic_cast<RobotBehaviour_MT*>(this->handControl);
//    RobotBehaviour_MT* thisB1 = this->handControl;
//    logger->debug("will jetz hctr starten");
//    if (thisB) {
//        logger->info("Starting module "+thisB->getId());
//        foreach (RobotBehaviour* b, behaviours) {
//            if (b != dynamic_cast<RobotBehaviour*>(thisB))
//                b->stop();
//        }
//        emit stopAllBehaviours();

//        thisB->start();
//        emit startHandCtr();
//    }
    RobotModule* thisB = dynamic_cast<RobotModule*>(this->handControl);
    logger->info("Try to start HandCtr");
    if(thisB)
    {
        logger->info("HandCtr ist available");
    }
    emit stopAllBehaviours();
    emit startHandCtr();
//    QTimer::singleShot(0,handControl,SLOT(start()));
    logger->info("Signal emitted");
}

void MetaBehaviour::depthChanged(float depth)
{
    if (getDataValue("state")=="dive" && fabs(tcl->getDepthError())<getSettingsValue("depthErrorVariance").toFloat()) {
        addData("state", "pipe");
        emit dataChanged(this);
        emit startPipeFollow();
//        QTimer::singleShot(0, pipe, SLOT(start()));
        timeoutTimer.stop();
        timeoutTimer.start(getSettingsValue("timeout").toInt()*1000);
    }
    if (getDataValue("state")=="diveSimple" && fabs(tcl->getDepthError())<getSettingsValue("depthErrorVariance").toFloat()) {
        addData("state", "forward");
        emit setForwardSpeed(getSettingsValue("forwardSpeed").toFloat());
//        emit setAngularSpeed(-0.5);
//        tcl->setForwardSpeed(getSettingsValue("forwardSpeed").toFloat());
//        tcl->setAngularSpeed(-0.05);
        timeoutTimer.stop();
        timeoutTimer.start(getSettingsValue("timeout").toInt()*1000);
    }
    if (getDataValue("state")=="diveForPipe" && fabs(tcl->getDepthError())<getSettingsValue("depthErrorVariance").toFloat()) {
        addData("state", "pipeFirstPart");
        emit startPipeFollow();
//        QTimer::singleShot(0, pipe, SLOT(start()));
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
//        tcl->setDepth(0);
    }
}

void MetaBehaviour::testPipe()
{
    addData("state", "pipe");
    emit dataChanged(this);
    emit startPipeFollow();
//    QTimer::singleShot(0, pipe, SLOT(start()));
    timeoutTimer.stop();
    timeoutTimer.start(getSettingsValue("timeout").toInt()*1000);
}

void MetaBehaviour::finishedTurn(RobotBehaviour_MT *, bool success) {
    if (getDataValue("state")=="turn") {
        addData("state","pipeSecondPart");
        emit dataChanged(this);
        emit startPipeFollow();
//        QTimer::singleShot(0, pipe, SLOT(start()));
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
//    tcl->setDepth(0);
//    tcl->setForwardSpeed(0);
//    tcl->setAngularSpeed(0);
}

void MetaBehaviour::badHealth(RobotModule *m)
{
    if (m==pressure) {
        addData("state","fail");
        emit dataChanged(this);
//        foreach (RobotBehaviour* b, behaviours) {
//            b->stop();
//        }
        emit stopAllBehaviours();
        emit setDepth(0);
//        tcl->setDepth(0);
        timeoutTimer.stop();
    }
}


void MetaBehaviour::pipeFollowForward()
{
    emergencyStop();

    addData("state","dive");
    emit dataChanged(this);
    emit setDepth(getSettingsValue("targetDepth").toFloat());
//    tcl->setDepth(getSettingsValue("targetDepth").toFloat());
    timeoutTimer.start(getSettingsValue("timeout").toInt()*1000);
}

void MetaBehaviour::simpleForward()
{
    emergencyStop();

    addData("state","diveSimple");
    emit dataChanged(this);
    emit setDepth(getSettingsValue("targetDepth").toFloat());
//    tcl->setDepth(getSettingsValue("targetDepth").toFloat());
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

//    QTimer::singleShot(0, o80, SLOT(start()));
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
//    tcl->setDepth(getSettingsValue("targetDepth").toFloat());
    timeoutTimer.start(getSettingsValue("timeout").toInt()*1000);
}

void MetaBehaviour::finishedPipe(RobotBehaviour_MT *, bool success) {
    if (getDataValue("state")=="pipe") {
        addData("state","surface");
        emit dataChanged(this);
        emit setDepth(0);
//        tcl->setDepth(0);
        timeoutTimer.stop();
    }
    if (getDataValue("state")=="pipeFirstPart") {
        addData("state","turn");
        addData("reachedVG", true);
        emit dataChanged(this);
        emit startTurnO80();
//        QTimer::singleShot(0, o80, SLOT(start()));
        timeoutTimer.start(getSettingsValue("timeout").toInt()*1000);
    }
    if (getDataValue("state")=="pipeSecondPart") {
        addData("state","ball");
        emit dataChanged(this);
        emit setDepth(1.5);
//        tcl->setDepth(1.5);
        emit startBallFollow();
//        QTimer::singleShot(0, ball, SLOT(start()));
        timeoutTimer.start(5*60*1000);
    }
}
