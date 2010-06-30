#include "behaviour_ballfollowing.h"

#include <Module_ThrusterControlLoop/module_thrustercontrolloop.h>
#include <QtGui>
#include <Behaviour_BallFollowing/ballfollowingform.h>

Behaviour_BallFollowing::Behaviour_BallFollowing(QString id, Module_ThrusterControlLoop *tcl, Module_VisualSLAM *vsl)
    : RobotBehaviour(id)
{
    this->tcl = tcl;
    this->vsl = vsl;
    connect(vsl,SIGNAL(foundNewObject(int)),this,SLOT(newData(int)));

    setEnabled(false);

}

bool Behaviour_BallFollowing::isActive()
{
    return isEnabled();
}

void Behaviour_BallFollowing::start()
{
    this->setEnabled(true);
}

void Behaviour_BallFollowing::newData(int classNr)
{
    if(this->isEnabled() && classNr == 2) Behaviour_BallFollowing::ctrBallFollowing();
}

void Behaviour_BallFollowing::stop()
{
    if (isEnabled()) {
       this->tcl->setForwardSpeed(0.0);
       this->tcl->setAngularSpeed(0.0);
       setEnabled(false);
       emit finished(this,false);
   }
}

void Behaviour_BallFollowing::reset()
{
    RobotBehaviour::reset();
    this->tcl->setForwardSpeed(0.0);
    this->tcl->setAngularSpeed(0.0);
}

QList<RobotModule*> Behaviour_BallFollowing::getDependencies()
{
    QList<RobotModule*> ret;
    ret.append(tcl);
    return ret;
}

QWidget* Behaviour_BallFollowing::createView(QWidget* parent)
{
    return new BallFollowingForm(parent, this);
}

void Behaviour_BallFollowing::ctrBallFollowing()
{
    QRectF rect;
    QDateTime current;
    vsl->getObjectPosition( 2, rect, current ); // 2 == BALLS
    float x = (rect.topLeft().x() + rect.topRight().x()) / 2;
    float robCenterX = this->getSettings().value("robCenterX").toFloat();
    float diff = robCenterX - x;
    float angleSpeed = 0.0;
    diff < 0.0 ? diff *= (-1) : diff;
    if(diff > this->getSettings().value("deltaBall").toFloat())
    {
        angleSpeed = this->getSettings().value("kpBall").toFloat() * ((robCenterX - x)/this->getSettings().value("maxDistance").toFloat());
    }
    tcl->setAngularSpeed(angleSpeed);
    tcl->setForwardSpeed(this->getSettings().value("fwSpeed").toFloat());
    timerNoBall.start(2000);

}

void Behaviour_BallFollowing::timerSlot()
{
    timerNoBall.stop();

}
