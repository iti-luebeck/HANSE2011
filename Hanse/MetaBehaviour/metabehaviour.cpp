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

    // find all behaviours
    foreach(RobotModule* m, graph->getModules()) {
        RobotBehaviour* b = dynamic_cast<RobotBehaviour*>(m);
        if (b)
            this->behaviours.append(b);
    }

    // states: dive; pipe; surface
    data["state"]="off";

    emit dataChanged(this);

    timeoutTimer.setSingleShot(true);
    connect(&timeoutTimer, SIGNAL(timeout()), this, SLOT(stateTimeout()));

    connect(pressure, SIGNAL(newDepthData(float)), this, SLOT(depthChanged(float)));
    connect(pipe, SIGNAL(finished(RobotBehaviour*,bool)), this, SLOT(finishedPipe(RobotBehaviour*,bool)));
    connect(o80, SIGNAL(finished(RobotBehaviour*,bool)), this, SLOT(finishedTurn(RobotBehaviour*,bool)));
    connect(pressure, SIGNAL(healthStatusChanged(RobotModule*)), this, SLOT(badHealth(RobotModule*)));

    connect(handControl, SIGNAL(emergencyStop()), this, SLOT(emergencyStop()));
    connect(handControl, SIGNAL(startHandControl()), this, SLOT(startHandControl()));
}

void MetaBehaviour::emergencyStop()
{
    foreach (RobotBehaviour* b, behaviours) {
        b->stop();
    }
    tcl->reset();

    data["state"]="off";
    timeoutTimer.stop();

}

QList<RobotModule*> MetaBehaviour::getDependencies()
{
    QList<RobotModule*> ret;
    return ret;
}

QWidget* MetaBehaviour::createView(QWidget* parent)
{
    return new MetaBehaviourForm(this, parent);
}

void MetaBehaviour::reset()
{

}

void MetaBehaviour::startHandControl()
{
    RobotBehaviour* thisB = this->handControl;
    if (thisB) {
        logger->info("Starting module "+thisB->getId());
        foreach (RobotBehaviour* b, behaviours) {
            if (b != thisB)
                b->stop();
        }
        thisB->start();
    }
}

void MetaBehaviour::depthChanged(float depth)
{
    if (data["state"]=="dive" && fabs(tcl->getDepthError())<settings.value("depthErrorVariance").toFloat()) {
        data["state"] = "pipe";
        emit dataChanged(this);
        QTimer::singleShot(0, pipe, SLOT(start()));
        timeoutTimer.stop();
        timeoutTimer.start(settings.value("timeout").toInt()*1000);
    }
    if (data["state"]=="diveSimple" && fabs(tcl->getDepthError())<settings.value("depthErrorVariance").toFloat()) {
        data["state"] = "forward";
        tcl->setForwardSpeed(settings.value("forwardSpeed").toFloat());
        tcl->setAngularSpeed(-0.05);
        timeoutTimer.stop();
        timeoutTimer.start(settings.value("timeout").toInt()*1000);
    }
    if (data["state"]=="diveForPipe" && fabs(tcl->getDepthError())<settings.value("depthErrorVariance").toFloat()) {
        data["state"] = "pipeFirstPart";
        QTimer::singleShot(0, pipe, SLOT(start()));
        timeoutTimer.stop();
        timeoutTimer.start(settings.value("timeout").toInt()*1000);
    }
    if (data["state"]=="surface" && fabs(tcl->getDepthError())<settings.value("depthErrorVariance").toFloat()) {
        data["state"] = "off";
        emit dataChanged(this);
        timeoutTimer.stop();
    }
    if (depth>3) {
        data["state"]="surface";
        emit dataChanged(this);
        timeoutTimer.stop();
        tcl->setDepth(0);
    }
}

void MetaBehaviour::testPipe()
{
    data["state"] = "pipe";
    emit dataChanged(this);
    QTimer::singleShot(0, pipe, SLOT(start()));
    timeoutTimer.stop();
    timeoutTimer.start(settings.value("timeout").toInt()*1000);
}

void MetaBehaviour::finishedTurn(RobotBehaviour *, bool success) {
    if (data["state"]=="turn") {
        data["state"]="pipeSecondPart";
        emit dataChanged(this);
        QTimer::singleShot(0, pipe, SLOT(start()));
        timeoutTimer.start(settings.value("timeout").toInt()*1000);
    }
}

void MetaBehaviour::stateTimeout()
{
    data["state"]="timeoutFail";
    emit dataChanged(this);
    emergencyStop();
    tcl->setDepth(0);
    tcl->setForwardSpeed(0);
    tcl->setAngularSpeed(0);
}

void MetaBehaviour::badHealth(RobotModule *m)
{
    if (m==pressure) {
        data["state"]="fail";
        emit dataChanged(this);
        foreach (RobotBehaviour* b, behaviours) {
            b->stop();
        }
        tcl->setDepth(0);
        timeoutTimer.stop();
    }
}


void MetaBehaviour::pipeFollowForward()
{
    emergencyStop();

    data["state"]="dive";
    emit dataChanged(this);
    tcl->setDepth(settings.value("targetDepth").toFloat());
    timeoutTimer.start(settings.value("timeout").toInt()*1000);
}

void MetaBehaviour::simpleForward()
{
    emergencyStop();

    data["state"]="diveSimple";
    emit dataChanged(this);
    tcl->setDepth(settings.value("targetDepth").toFloat());
    timeoutTimer.start(settings.value("timeout").toInt()*1000);
}

void MetaBehaviour::simple180deg()
{
    emergencyStop();


        QTimer::singleShot(0, o80, SLOT(initialHeadingUpdate()));
        msleep(10000);

    data["state"]="180simple";
    emit dataChanged(this);
    timeoutTimer.start(settings.value("timeout").toInt()*1000);

    QTimer::singleShot(0, o80, SLOT(start()));
}

void MetaBehaviour::fullProgram()
{
    emergencyStop();
    data["reachedVG"] = false;

    QTimer::singleShot(0, o80, SLOT(initialHeadingUpdate()));

    data["state"]="diveForPipe";
    emit dataChanged(this);
    tcl->setDepth(settings.value("targetDepth").toFloat());
    timeoutTimer.start(settings.value("timeout").toInt()*1000);
}

void MetaBehaviour::finishedPipe(RobotBehaviour *, bool success) {
    if (data["state"]=="pipe") {
        data["state"]="surface";
        emit dataChanged(this);
        tcl->setDepth(0);
        timeoutTimer.stop();
    }
    if (data["state"]=="pipeFirstPart") {
        data["state"]="turn";
        data["reachedVG"] = true;
        emit dataChanged(this);
        QTimer::singleShot(0, o80, SLOT(start()));
        timeoutTimer.start(settings.value("timeout").toInt()*1000);
    }
    if (data["state"]=="pipeSecondPart") {
        data["state"]="ball";
        emit dataChanged(this);
        tcl->setDepth(1.5);
        QTimer::singleShot(0, ball, SLOT(start()));
        timeoutTimer.start(5*60*1000);
    }
}
