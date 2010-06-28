#include "metabehaviour.h"

#include <Module_ThrusterControlLoop/module_thrustercontrolloop.h>
#include <Module_HandControl/module_handcontrol.h>
#include <Framework/modulesgraph.h>
#include <QtGui>
#include "metabehaviourform.h"

MetaBehaviour::MetaBehaviour(QString id, ModulesGraph* graph, Module_ThrusterControlLoop* tcl, Module_HandControl* handControl)
    : RobotModule(id)
{
    this->tcl = tcl;
    this->handControl = handControl;

    // find all behaviours
    foreach(RobotModule* m, graph->getModules()) {
        RobotBehaviour* b = dynamic_cast<RobotBehaviour*>(m);
        if (b)
            this->behaviours.append(b);
    }

    connect(handControl, SIGNAL(emergencyStop()), this, SLOT(emergencyStop()));
    connect(handControl, SIGNAL(startHandControl()), this, SLOT(startHandControl()));
}

void MetaBehaviour::emergencyStop()
{
    foreach (RobotBehaviour* b, behaviours) {
        b->stop();
    }
    tcl->reset();

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
