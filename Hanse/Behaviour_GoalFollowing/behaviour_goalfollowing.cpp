#include "behaviour_goalfollowing.h"

#include <Module_ThrusterControlLoop/module_thrustercontrolloop.h>
#include <QtGui>
#include <Behaviour_GoalFollowing/goalfollowingform.h>

Behaviour_GoalFollowing::Behaviour_GoalFollowing(QString id, Module_ThrusterControlLoop *tcl, Module_VisualSLAM *vsl)
    : RobotBehaviour(id)
{
    this->tcl = tcl;
    this->vsl = vsl;
    connect(&robMod,SIGNAL(dataChanged(RobotModule*)),this,SLOT(newData()));

    setEnabled(false);

}

bool Behaviour_GoalFollowing::isActive()
{
    return isEnabled();
}

void Behaviour_GoalFollowing::start()
{
    this->setEnabled(true);
}

void Behaviour_GoalFollowing::newData()
{
    if(this->isEnabled()) Behaviour_GoalFollowing::ctrGoalFollowing();
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
    Behaviour_GoalFollowing::stop();
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
    QRectF rect;
    QDateTime current;
    vsl->getObjectPosition( 1, rect, current );

}
