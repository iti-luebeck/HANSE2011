#include "modulesgraph.h"

#include <Module_ScanningSonar/module_scanningsonar.h>
#include <Module_UID/module_uid.h>
#include <Module_Thruster/module_thruster.h>
#include <Module_PressureSensor/module_pressuresensor.h>
#include <Module_ThrusterControlLoop/module_thrustercontrolloop.h>
#include <Module_HandControl/module_handcontrol.h>
#include <Module_IMU/module_imu.h>
#include <Module_Compass/module_compass.h>
#include <Module_SonarLocalization/module_sonarlocalization.h>
#include <Module_VisualSLAM/module_visualslam.h>
#include <Module_Localization/module_localization.h>
#include <Module_Navigation/module_navigation.h>

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

    Module_IMU* imu = new Module_IMU("adis",uid);
    this->modules.append(imu);

    Module_Compass *compass = new Module_Compass("compass", uid);
    this->modules.append(compass);

    Module_ThrusterControlLoop* controlLoop = new Module_ThrusterControlLoop("controlLoop",pressure, thrusterLeft, thrusterRight, thrusterDown);
    this->modules.append(controlLoop);

    Module_HandControl* handControl = new Module_HandControl("handControl",controlLoop, thrusterLeft, thrusterRight, thrusterDown);
    this->modules.append(handControl);

    Module_SonarLocalization* sonarLoc = new Module_SonarLocalization("sonarLocalize", sonar);
    this->modules.append(sonarLoc);

    Module_VisualSLAM* visualLoc = new Module_VisualSLAM("visualSLAM", sonarLoc);
    this->modules.append(visualLoc);

    Module_Localization* local = new Module_Localization("localization",visualLoc, sonarLoc,pressure);
    this->modules.append(local);

    Module_Navigation* navi = new Module_Navigation("navigation",local, controlLoop);
    this->modules.append(navi);

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
