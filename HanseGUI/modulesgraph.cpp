#include "modulesgraph.h"

#include <module_scanningsonar.h>
#include <module_uid.h>
#include <module_thruster.h>
#include <module_pressuresensor.h>
#include <module_thrustercontrolloop.h>
#include <module_handcontrol.h>

ModulesGraph::ModulesGraph()
{
    logger = Log4Qt::Logger::logger("ModulesGraph");
}

void ModulesGraph::build()
{

    logger->info("Loading all Modules...");

    Module_UID* uid = new Module_UID("uid");
    this->modules.append(uid);

    Module_ScanningSonar* sonar = new Module_ScanningSonar("sonar");
    this->modules.append(sonar);

    Module_Thruster* thrusterRight = new Module_Thruster("thrusterRight",uid);
    this->modules.append(thrusterRight);
    Module_Thruster* thrusterLeft = new Module_Thruster("thrusterLeft",uid);
    this->modules.append(thrusterLeft);
    Module_Thruster* thrusterDown = new Module_Thruster("thrusterDown",uid);
    this->modules.append(thrusterDown);

    Module_PressureSensor* pressure = new Module_PressureSensor("pressure",uid);
    this->modules.append(pressure);

    Module_ThrusterControlLoop* controlLoop = new Module_ThrusterControlLoop("controlLoop",pressure, thrusterLeft, thrusterRight, thrusterDown);
    this->modules.append(controlLoop);

    Module_HandControl* handControl = new Module_HandControl("handControl",controlLoop, thrusterLeft, thrusterRight, thrusterDown);
    this->modules.append(handControl);

    logger->info("Loading all Modules... Done");
}

QList<RobotModule*> ModulesGraph::getModules()
{
    return QList<RobotModule*>(modules);
}

void ModulesGraph::HastaLaVista()
{
    logger->info("Terminating all modules...");
    // go backwards through the module list
    for (int i = modules.size()-1; i>=0; i--) {
        logger->info("Terminating "+modules[i]->getId());
        modules[i]->terminate();
    }
    logger->info("All modules terminated.");
}
