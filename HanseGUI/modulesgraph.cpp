#include "modulesgraph.h"

#include <module_scanningsonar.h>
#include <module_uid.h>

ModulesGraph::ModulesGraph()
{
    logger = Log4Qt::Logger::logger("ModulesGraph");

    build();
}

void ModulesGraph::build()
{

    logger->info("Loading all Modules...");

    Module_UID* uid1 = new Module_UID("uid0","UID0001");
    this->modules.append(uid1);

    Module_ScanningSonar* sonar = new Module_ScanningSonar("sonar0");
    this->modules.append(sonar);

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
    for (int i = modules.size()-1; i==0; i--) {
        modules[i]->terminate();
    }
    logger->info("All modules terminated.");
}
