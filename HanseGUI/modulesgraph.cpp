#include "modulesgraph.h"

#include <module_scanningsonar.h>
#include <module_serialport.h>
ModulesGraph::ModulesGraph()
{
    build();
}

void ModulesGraph::build()
{

    Module_SerialPort* serialPort1 = new Module_SerialPort("serial1");
    this->modules.append(serialPort1);

    Module_ScanningSonar* sonar = new Module_ScanningSonar("sonar0", serialPort1);
    this->modules.append(sonar);
}

QList<RobotModule*> ModulesGraph::getModules()
{
    return QList<RobotModule*>(modules);
}
