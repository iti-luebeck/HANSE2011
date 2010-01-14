#include "modulesgraph.h"

#include <module_scanningsonar.h>
ModulesGraph::ModulesGraph()
{
    logger = Log4Qt::Logger::logger("ModulesGraph");

    build();
}

void ModulesGraph::build()
{

    logger->info("Loading all Modules...");

    //Module_SerialPort* serialPort1 = new Module_SerialPort("serial1");
    //this->modules.append(serialPort1);

    Module_ScanningSonar* sonar = new Module_ScanningSonar("sonar0");
    this->modules.append(sonar);

    logger->info("Loading all Modules... Done");
}

QList<RobotModule*> ModulesGraph::getModules()
{
    return QList<RobotModule*>(modules);
}
