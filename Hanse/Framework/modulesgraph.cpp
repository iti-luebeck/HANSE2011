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
#include <Behaviour_BallFollowing/behaviour_ballfollowing.h>
#include <Module_Webcams/module_webcams.h>
#include <MetaBehaviour/metabehaviour.h>
#include <Behaviour_TurnOneEighty/behaviour_turnoneeighty.h>

ModulesGraph::ModulesGraph()
{
    logger = Log4Qt::Logger::logger("ModulesGraph");
}

void ModulesGraph::build()
{
    logger->info("Loading all Modules...");

    Module_UID* uid = new Module_UID("uid");
    this->modules.append(uid);
    
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

    logger->debug("Creating Module_ScanningSonar");
    Module_ScanningSonar* sonar = new Module_ScanningSonar("sonar",controlLoop);
    this->modules.append(sonar);

    logger->debug("Creating Module_HandControl");
    Module_HandControl* handControl = new Module_HandControl("handControl",controlLoop, thrusterLeft, thrusterRight, thrusterDown, thrusterDownF);
    this->modules.append(handControl);

    logger->debug("Creating Module_SonarLocalization");
    Module_SonarLocalization* sonarLoc = new Module_SonarLocalization("sonarLocalize", sonar, pressure);
    this->modules.append(sonarLoc);

    logger->debug("Creating Module_Webcams");
    Module_Webcams *cams = new Module_Webcams( "cams" );
    this->modules.append( cams );

    logger->debug("Creating Module_VisualSLAM");
    Module_VisualSLAM* visualLoc = new Module_VisualSLAM( "visualSLAM", sonarLoc, cams );
    this->modules.append(visualLoc);

    logger->debug("Creating Module_Navigation");
    Module_Navigation* navi = new Module_Navigation( "navigation", sonarLoc, visualLoc, controlLoop, pressure, compass );
    this->modules.append(navi);

    logger->debug("Creating Behaviour_PipeFollowing");
    Behaviour_PipeFollowing* behavPipe = new Behaviour_PipeFollowing("pipe",controlLoop,cams);
    this->modules.append(behavPipe);

    logger->debug("Creating Behaviour_GoalFollowing");
    Behaviour_GoalFollowing* behavGoal = new Behaviour_GoalFollowing("goal",controlLoop, visualLoc);
    this->modules.append(behavGoal);

    logger->debug("Creating Behaviour_BallFollowing");
    Behaviour_BallFollowing* behavBall = new Behaviour_BallFollowing("ball",controlLoop, cams, compass);
    this->modules.append(behavBall);

    logger->debug("Creating Behaviour_TurnOneEighty");
    Behaviour_TurnOneEighty* behavTurn = new Behaviour_TurnOneEighty("turn",controlLoop, compass);
    this->modules.append(behavTurn);

    // IMPORTANT: must be the last module to be loaded, otherwise it won't have access to all the other modules
    logger->debug("Creating MetaBehaviour");
    MetaBehaviour* metaBehaviour = new MetaBehaviour("meta",this, controlLoop, handControl, pressure, behavPipe, behavBall, behavTurn);
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
