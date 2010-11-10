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
}

void MetaBehaviour::emergencyStop()
{
    foreach (RobotBehaviour* b, behaviours) {
        b->stop();
    }
    tcl->reset();

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

void MetaBehaviour::startHandControl()
{
    RobotBehaviour* thisB = dynamic_cast<RobotBehaviour*>(this->handControl);
    RobotBehaviour_MT* thisB1 = this->handControl;
    logger->debug("will jetz hctr starten");
    if (thisB1) {
        logger->info("Starting module "+thisB1->getId());
        foreach (RobotBehaviour* b, behaviours) {
            if (b != dynamic_cast<RobotBehaviour*>(thisB1))
                b->stop();
        }
        thisB1->start();
    }
}

void MetaBehaviour::depthChanged(float depth)
{
    if (getDataValue("state")=="dive" && fabs(tcl->getDepthError())<getSettingsValue("depthErrorVariance").toFloat()) {
        addData("state", "pipe");
        emit dataChanged(this);
        QTimer::singleShot(0, pipe, SLOT(start()));
        timeoutTimer.stop();
        timeoutTimer.start(getSettingsValue("timeout").toInt()*1000);
    }
    if (getDataValue("state")=="diveSimple" && fabs(tcl->getDepthError())<getSettingsValue("depthErrorVariance").toFloat()) {
        addData("state", "forward");
        tcl->setForwardSpeed(getSettingsValue("forwardSpeed").toFloat());
        tcl->setAngularSpeed(-0.05);
        timeoutTimer.stop();
        timeoutTimer.start(getSettingsValue("timeout").toInt()*1000);
    }
    if (getDataValue("state")=="diveForPipe" && fabs(tcl->getDepthError())<getSettingsValue("depthErrorVariance").toFloat()) {
        addData("state", "pipeFirstPart");
        QTimer::singleShot(0, pipe, SLOT(start()));
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
        tcl->setDepth(0);
    }
}

void MetaBehaviour::testPipe()
{
    addData("state", "pipe");
    emit dataChanged(this);
    QTimer::singleShot(0, pipe, SLOT(start()));
    timeoutTimer.stop();
    timeoutTimer.start(getSettingsValue("timeout").toInt()*1000);
}

void MetaBehaviour::finishedTurn(RobotBehaviour *, bool success) {
    if (getDataValue("state")=="turn") {
        addData("state","pipeSecondPart");
        emit dataChanged(this);
        QTimer::singleShot(0, pipe, SLOT(start()));
        timeoutTimer.start(getSettingsValue("timeout").toInt()*1000);
    }
}

void MetaBehaviour::stateTimeout()
{
    addData("state","timeoutFail");
    emit dataChanged(this);
    emergencyStop();
    tcl->setDepth(0);
    tcl->setForwardSpeed(0);
    tcl->setAngularSpeed(0);
}

void MetaBehaviour::badHealth(RobotModule *m)
{
    if (m==pressure) {
        addData("state","fail");
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

    addData("state","dive");
    emit dataChanged(this);
    tcl->setDepth(getSettingsValue("targetDepth").toFloat());
    timeoutTimer.start(getSettingsValue("timeout").toInt()*1000);
}

void MetaBehaviour::simpleForward()
{
    emergencyStop();

    addData("state","diveSimple");
    emit dataChanged(this);
    tcl->setDepth(getSettingsValue("targetDepth").toFloat());
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

    QTimer::singleShot(0, o80, SLOT(start()));
}

void MetaBehaviour::fullProgram()
{
    emergencyStop();
    addData("reachedVG", false);

    QTimer::singleShot(0, o80, SLOT(initialHeadingUpdate()));

    addData("state","diveForPipe");
    emit dataChanged(this);
    tcl->setDepth(getSettingsValue("targetDepth").toFloat());
    timeoutTimer.start(getSettingsValue("timeout").toInt()*1000);
}

void MetaBehaviour::finishedPipe(RobotBehaviour *, bool success) {
    if (getDataValue("state")=="pipe") {
        addData("state","surface");
        emit dataChanged(this);
        tcl->setDepth(0);
        timeoutTimer.stop();
    }
    if (getDataValue("state")=="pipeFirstPart") {
        addData("state","turn");
        addData("reachedVG", true);
        emit dataChanged(this);
        QTimer::singleShot(0, o80, SLOT(start()));
        timeoutTimer.start(getSettingsValue("timeout").toInt()*1000);
    }
    if (getDataValue("state")=="pipeSecondPart") {
        addData("state","ball");
        emit dataChanged(this);
        tcl->setDepth(1.5);
        QTimer::singleShot(0, ball, SLOT(start()));
        timeoutTimer.start(5*60*1000);
    }
}
