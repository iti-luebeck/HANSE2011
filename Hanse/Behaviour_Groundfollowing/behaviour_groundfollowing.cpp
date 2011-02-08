#include "behaviour_groundfollowing.h"
#include <Module_EchoSounder/module_echosounder.h>

#include <Module_ThrusterControlLoop/module_thrustercontrolloop.h>
#include <QtGui>
#include <Behaviour_GroundFollowing/groundfollowingform.h>
//#include <OpenCV/include/opencv/cv.h>
#include <opencv/cxcore.h>

Behaviour_GroundFollowing::Behaviour_GroundFollowing(QString id, Module_ThrusterControlLoop *tcl)
    : RobotBehaviour(id)
{
    this->tcl = tcl;
    connect(&timer,SIGNAL(timeout()),this,SLOT(timerSlot()));
    connect (this,SIGNAL(timerStart(int)),&timer,SLOT(start(int)));
    connect(this, SIGNAL(timerStop()),&timer,SLOT(stop()));
}

bool Behaviour_GroundFollowing::isActive(){
    return isEnabled();
}

void Behaviour_GroundFollowing::start(){

}

void Behaviour_GroundFollowing::stop(){

}

void Behaviour_GroundFollowing::terminate(){

}

void Behaviour_GroundFollowing::reset(){

}

void Behaviour_GroundFollowing::updateFromSettings(){

}

void Behaviour_GroundFollowing::updateData(){

}

void Behaviour_GroundFollowing::setValues(float distance, int range){
    this->distance = distance;
    this->range = range;
}

void Behaviour_GroundFollowing::timerSlot(){

}

QWidget* Behaviour_GroundFollowing::createView(QWidget* parent)
{
    return new GroundFollowingForm( parent, this);
}
