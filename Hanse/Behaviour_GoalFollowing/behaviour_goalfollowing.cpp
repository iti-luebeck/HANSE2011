#include "behaviour_goalfollowing.h"

#include <Module_ThrusterControlLoop/module_thrustercontrolloop.h>
#include <QtGui>
#include <Behaviour_GoalFollowing/goalfollowingform.h>

Behaviour_GoalFollowing::Behaviour_GoalFollowing(QString id, Module_ThrusterControlLoop *tcl, Module_VisualSLAM *vsl)
    : RobotBehaviour(id)
{
    this->tcl = tcl;
    this->vsl = vsl;

    connect(vsl, SIGNAL(foundNewObject(int)),this,SLOT(newData(int)) );
    connect(&timerNoGoal, SIGNAL(timeout()),this,SLOT(timerSlot()));

    state = STATE_IDLE;

    setEnabled(false);

}

bool Behaviour_GoalFollowing::isActive()
{
    return isEnabled();
}

void Behaviour_GoalFollowing::start()
{
    this->setEnabled(true);
    timerNoGoal.start(10000);
}

void Behaviour_GoalFollowing::newData(int classNr)
{
    if(this->isEnabled() && classNr == 1) Behaviour_GoalFollowing::ctrGoalFollowing();
}

void Behaviour_GoalFollowing::stop()
{
    if (isEnabled()) {
       this->tcl->setForwardSpeed(0.0);
       this->tcl->setAngularSpeed(0.0);
       setEnabled(false);
       emit finished(this,false);
   }
}

void Behaviour_GoalFollowing::reset()
{
    RobotBehaviour::reset();
    this->tcl->setAngularSpeed(0.0);
    this->tcl->setForwardSpeed(0.0);
}

QList<RobotModule*> Behaviour_GoalFollowing::getDependencies()
{
    QList<RobotModule*> ret;
    ret.append(tcl);
    return ret;
}

QWidget* Behaviour_GoalFollowing::createView(QWidget* parent)
{
    return new GoalFollowingForm(parent, this);
}

void Behaviour_GoalFollowing::ctrGoalFollowing()
{
    timerNoGoal.stop();
    QRectF rect;
    QDateTime current;
    vsl->getObjectPosition( 1, rect, current );
    float x = (rect.topLeft().x() + rect.topRight().x()) / 2;
    float robCenterX = this->getSettings().value("robCenterX").toFloat();
    float diff = robCenterX - x;
    float angleSpeed = 0.0;
    diff < 0.0 ? diff *= (-1) : diff;
    if(diff > this->getSettings().value("deltaGoal").toFloat())
    {
        angleSpeed = this->getSettings().value("kpGoal").toFloat() * ((robCenterX - x)/this->getSettings().value("maxDistance").toFloat());
    }
    tcl->setAngularSpeed(angleSpeed);
    tcl->setForwardSpeed(this->getSettings().value("fwSpeed").toFloat());

    state = STATE_SEEN_GOAL;

    timerNoGoal.start(2000);
    emit started(this);
}

void Behaviour_GoalFollowing::timerSlot()
{
    timerNoGoal.stop();
    switch(this->state)
    {
    case STATE_SEEN_GOAL:
        state = STATE_FORWARD;
        tcl->setAngularSpeed( .0 );
        timerNoGoal.start(2000);
        break;
    case STATE_FORWARD:
        state = STATE_TURNING;
        tcl->setForwardSpeed( .0 );
        tcl->setAngularSpeed( .1 );
        timerNoGoal.start(2000);
        break;
    case STATE_TURNING:
        state = STATE_FAILED;
        this->setHealthToSick("no goal in sight");
         emit finished(this,false);
        break;
    }
}
