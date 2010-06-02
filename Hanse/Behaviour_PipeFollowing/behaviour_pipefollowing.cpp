#include "behaviour_pipefollowing.h"

#include <Module_ThrusterControlLoop/module_thrustercontrolloop.h>
#include <QtGui>

Behaviour_PipeFollowing::Behaviour_PipeFollowing(QString id, Module_ThrusterControlLoop *tcl)
    : RobotBehaviour(id)
{
    this->tcl = tcl;

    // TODO
}

bool Behaviour_PipeFollowing::isActive()
{
    return true; // TODO
}

void Behaviour_PipeFollowing::start()
{
    // TODO
}

void Behaviour_PipeFollowing::stop()
{
    // TODO
}

QList<RobotModule*> Behaviour_PipeFollowing::getDependencies()
{
    QList<RobotModule*> ret;
    ret.append(tcl);
    return ret;
}

QWidget* Behaviour_PipeFollowing::createView(QWidget* parent)
{
    return new QWidget(parent);
}
