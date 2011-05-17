#include "modulesgraph.h"

#include <Module_ScanningSonar/module_scanningsonar.h>
#include <Module_UID/module_uid.h>
#include <Module_Thruster/module_thruster.h>
#include <Module_PressureSensor/module_pressuresensor.h>
#include <Module_ThrusterControlLoop/module_thrustercontrolloop.h>
#include <Module_HandControl/module_handcontrol.h>
#include <Module_Compass/module_compass.h>
#include <Module_SonarLocalization/module_sonarlocalization.h>
#include <Module_Navigation/module_navigation.h>
#include <Behaviour_PipeFollowing/behaviour_pipefollowing.h>
#include <Behaviour_GoalFollowing/behaviour_goalfollowing.h>
#include <Behaviour_BallFollowing/behaviour_ballfollowing.h>
#include <Module_Webcams/module_webcams.h>
#include <Behaviour_TurnOneEighty/behaviour_turnoneeighty.h>
#include <Behaviour_CompassFollowing/behaviour_compassfollowing.h>
#include <Module_Simulation/module_simulation.h>
#include <Module_EchoSounder/module_echosounder.h>
#include <Module_XsensMTi/module_xsensmti.h>
#include <Behaviour_WallFollowing/behaviour_wallfollowing.h>
#include <CommandCenter/commandcenter.h>
#include "SoToSleep.h"
#include <TaskHandControl/taskhandcontrol.h>
#include <TaskWallNavigation/taskwallnavigation.h>
#include <Behaviour_XsensFollowing/behaviour_xsensfollowing.h>
//#include <Module_IMU/module_imu.h>
//#include <Module_ADC/module_adc.h>

ModulesGraph::ModulesGraph()
{
    logger = Log4Qt::Logger::logger("ModulesGraph");
}

void ModulesGraph::build()
{
    logger->info("Loading all Modules...");

    //    logger->debug("Starting Simulation");
    Module_Simulation* sim = new Module_Simulation("simulation");
    this->modules.append(sim);


    Module_UID* uid = new Module_UID("uid");
    this->modules.append(uid);

    Module_Thruster* thrusterRight = new Module_Thruster("thrusterRight",uid,sim);
    this->modules.append(thrusterRight);
    Module_Thruster* thrusterLeft = new Module_Thruster("thrusterLeft",uid,sim);
    this->modules.append(thrusterLeft);
    Module_Thruster* thrusterDown = new Module_Thruster("thrusterDown",uid,sim);
    this->modules.append(thrusterDown);
    Module_Thruster* thrusterDownF = new Module_Thruster("thrusterDownFront",uid,sim);
    this->modules.append(thrusterDownF);

    Module_PressureSensor* pressure = new Module_PressureSensor("pressure",uid,sim);
    this->modules.append(pressure);

//    Module_IMU* imu = new Module_IMU("adis",uid,sim);
//    this->modules.append(imu);

    Module_Compass *compass = new Module_Compass("compass", uid,sim);
    this->modules.append(compass);

    Module_XsensMTi *xsens = new Module_XsensMTi("xsens", sim);
    this->modules.append(xsens);

    Module_ThrusterControlLoop* controlLoop = new Module_ThrusterControlLoop("controlLoop",pressure, thrusterLeft, thrusterRight, thrusterDown,thrusterDownF);
    this->modules.append(controlLoop);

    //    Module_ADC* adc0 = new Module_ADC("adc0",uid);
    //    this->modules.append(adc0);

    //    Module_ADC* adc1 = new Module_ADC("adc1",uid);
    //    this->modules.append(adc1);

    logger->debug("Creating Module_ScanningSonar");
    Module_ScanningSonar* sonar = new Module_ScanningSonar("sonar", sim);
    this->modules.append(sonar);

    logger->debug("Creating Module_EchoSounder");
    Module_EchoSounder* echo = new Module_EchoSounder("echo",sim);
    this->modules.append(echo);


    logger->debug("Creating Module_HandControl");
    Module_HandControl* handControl = new Module_HandControl("handControl",controlLoop, thrusterLeft, thrusterRight, thrusterDown, thrusterDownF);
    this->modules.append(handControl);

    logger->debug("Creating Module_Webcams");
    Module_Webcams *cams = new Module_Webcams( "cams" );
    this->modules.append( cams );

    logger->debug("Creating Module_SonarLocalization");
    Module_SonarLocalization* sonarLoc = new Module_SonarLocalization("sonarLocalize", sonar, xsens, sim);
    this->modules.append(sonarLoc);

    logger->debug("Creating Module_Navigation");
    Module_Navigation* navi = new Module_Navigation( "navigation", sonarLoc, controlLoop, pressure, compass, xsens );
    this->modules.append(navi);

    logger->debug("Creating Behaviour_PipeFollowing");
    Behaviour_PipeFollowing* behavPipe = new Behaviour_PipeFollowing("pipe",controlLoop,cams,sim);
    this->modules.append(behavPipe);

    //    logger->debug("Creating Behaviour_GoalFollowing");
    //    Behaviour_GoalFollowing* behavGoal = new Behaviour_GoalFollowing("goal",controlLoop, visualLoc);
    //    this->modules.append(behavGoal);

    logger->debug("Creating Behaviour_BallFollowing");
    Behaviour_BallFollowing* behavBall = new Behaviour_BallFollowing("ball",controlLoop, cams, compass);
    this->modules.append(behavBall);

    logger->debug("Creating Behaviour_TurnOneEighty");
    Behaviour_TurnOneEighty* behavTurn = new Behaviour_TurnOneEighty("turn",controlLoop, compass, xsens);
    this->modules.append(behavTurn);

    logger->debug("Creating Behaviour_WallFollowing");
    Behaviour_WallFollowing* behavWall = new Behaviour_WallFollowing("wall",controlLoop, echo);
    this->modules.append(behavWall);

//        logger->debug("Creating Behaviour_CompassFollowing");
//        Behaviour_CompassFollowing* behavComp = new Behaviour_CompassFollowing("compFollow",controlLoop, compass);
//        this->modules.append(behavComp);

    logger->debug("Creating Behaviour_XsensFollowing");
    Behaviour_XsensFollowing* behavXsens = new Behaviour_XsensFollowing("xsensFollow",controlLoop, xsens);
    this->modules.append(behavXsens);


    logger->debug("Creating TaskWallNavigation");
    TaskWallNavigation *taskwallnavigation = new TaskWallNavigation("taskWallNavi",sim, behavWall, navi);
    this->modules.append(taskwallnavigation);

    logger->debug("Creating TaskHandControl");
    TaskHandControl *taskhandcontrol = new TaskHandControl("taskHand", controlLoop, sim, handControl);
    this->modules.append(taskhandcontrol);

    logger->debug("Creating CommandCenter");
    CommandCenter* commCent = new CommandCenter("comandCenter", controlLoop, handControl, pressure, sim, behavPipe, behavBall, behavTurn, behavWall, behavXsens, taskhandcontrol, taskwallnavigation);
    this->modules.append(commCent);

    logger->info("Loading all Modules... Done");

    /* connect every modul to healtCheckTimer */
    logger->debug("Starting threads...");
    foreach (RobotModule* b, modules)
    {
        b->start();
        while(!b->isInitialized())
            sotoSleep::msleep(10);

    }


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
        modules[i]->shutdown();
    }
    logger->info("All modules terminated.");
}
