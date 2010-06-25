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
#include <Module_Navigation/module_navigation.h>
#include <Behaviour_PipeFollowing/behaviour_pipefollowing.h>
#include <Behaviour_GoalFollowing/behaviour_goalfollowing.h>
#include <MetaBehaviour/metabehaviour.h>

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
    Module_Thruster* thrusterDownF = new Module_Thruster("thrusterDownFront",uid);
    this->modules.append(thrusterDownF);

    Module_PressureSensor* pressure = new Module_PressureSensor("pressure",uid);
    this->modules.append(pressure);

    Module_IMU* imu = new Module_IMU("adis",uid);
    this->modules.append(imu);

    Module_Compass *compass = new Module_Compass("compass", uid);
    this->modules.append(compass);

    Module_ThrusterControlLoop* controlLoop = new Module_ThrusterControlLoop("controlLoop",pressure, thrusterLeft, thrusterRight, thrusterDown,thrusterDownF);
    this->modules.append(controlLoop);

    Module_HandControl* handControl = new Module_HandControl("handControl",controlLoop, thrusterLeft, thrusterRight, thrusterDown, thrusterDownF);
    this->modules.append(handControl);

    Module_SonarLocalization* sonarLoc = new Module_SonarLocalization("sonarLocalize", sonar);
    this->modules.append(sonarLoc);

    Module_VisualSLAM* visualLoc = new Module_VisualSLAM("visualSLAM", sonarLoc);
    this->modules.append(visualLoc);

    Module_Navigation* navi = new Module_Navigation( "navigation", sonarLoc, visualLoc, controlLoop, pressure, compass );
    this->modules.append(navi);

    Behaviour_PipeFollowing* behavPipe = new Behaviour_PipeFollowing("pipe",controlLoop);
    this->modules.append(behavPipe);

    Behaviour_GoalFollowing* behavGoal = new Behaviour_GoalFollowing("goal",controlLoop, visualLoc);
    this->modules.append(behavGoal);

    MetaBehaviour* metaBehaviour = new MetaBehaviour("meta",this, controlLoop, handControl);
    this->modules.append(metaBehaviour);



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
