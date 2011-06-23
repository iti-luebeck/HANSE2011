#include "modulesgraph.h"

#include <Module_ScanningSonar/module_scanningsonar.h>
#include <Module_UID/module_uid.h>
#include <Module_Thruster/module_thruster.h>
#include <Module_PressureSensor/module_pressuresensor.h>
#include <Module_ThrusterControlLoop/module_thrustercontrolloop.h>
#include <Module_HandControl/module_handcontrol.h>
#include <Module_SonarLocalization/module_sonarlocalization.h>
#include <Module_Navigation/module_navigation.h>
#include <Behaviour_PipeFollowing/behaviour_pipefollowing.h>
#include <Behaviour_BallFollowing/behaviour_ballfollowing.h>
#include <Module_Webcams/module_webcams.h>
#include <Behaviour_TurnOneEighty/behaviour_turnoneeighty.h>
#include <Module_Simulation/module_simulation.h>
#include <Module_EchoSounder/module_echosounder.h>
#include <Module_XsensMTi/module_xsensmti.h>
#include <Behaviour_WallFollowing/behaviour_wallfollowing.h>
#include <CommandCenter/commandcenter.h>
#include "SoToSleep.h"
#include <TaskHandControl/taskhandcontrol.h>
#include <TaskWallFollowing/taskwallfollowing.h>
#include <Behaviour_XsensFollowing/behaviour_xsensfollowing.h>
//#include <Module_IMU/module_imu.h>
//#include <Module_ADC/module_adc.h>
#include <TaskXsensNavigation/taskxsensnavigation.h>
#include <TaskPipeFollowing/taskpipefollowing.h>
#include <TaskMidwaterTarget/taskmidwatertarget.h>

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

    Module_XsensMTi *xsens = new Module_XsensMTi("xsens", sim);
    this->modules.append(xsens);

    Module_ThrusterControlLoop* controlLoop = new Module_ThrusterControlLoop("controlLoop",pressure, thrusterLeft, thrusterRight, thrusterDown,thrusterDownF);
    this->modules.append(controlLoop);

    //    Module_ADC* adc0 = new Module_ADC("adc0",uid);
    //    this->modules.append(adc0);

    //    Module_ADC* adc1 = new Module_ADC("adc1",uid);
    //    this->modules.append(adc1);

    logger->debug("Creating Module_ScanningSonar");
    Module_ScanningSonar* sonar = new Module_ScanningSonar("sonar", sim, xsens);
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
    Module_Navigation* navi = new Module_Navigation( "navigation", sonarLoc, controlLoop, pressure, xsens );
    this->modules.append(navi);

    logger->debug("Creating Behaviour_PipeFollowing");
    Behaviour_PipeFollowing* behavPipe = new Behaviour_PipeFollowing("pipe", controlLoop, cams, sim);
    this->modules.append(behavPipe);

    logger->debug("Creating Behaviour_BallFollowing");
    Behaviour_BallFollowing* behavBall = new Behaviour_BallFollowing("ball",controlLoop, cams, xsens, sim);
    this->modules.append(behavBall);

    logger->debug("Creating Behaviour_TurnOneEighty");
    Behaviour_TurnOneEighty* behavTurn = new Behaviour_TurnOneEighty("turn",controlLoop, xsens);
    this->modules.append(behavTurn);

    logger->debug("Creating Behaviour_WallFollowing");
    Behaviour_WallFollowing* behavWall = new Behaviour_WallFollowing("wall",controlLoop, echo, xsens);
    this->modules.append(behavWall);

    logger->debug("Creating Behaviour_XsensFollowing");
    Behaviour_XsensFollowing* behavXsens = new Behaviour_XsensFollowing("xsensFollow",controlLoop, xsens);
    this->modules.append(behavXsens);

    logger->debug("Creating TaskWallFollowing");
    TaskWallFollowing *taskwallfollowing = new TaskWallFollowing("taskWallFollow" ,behavWall, sim, navi, behavTurn);
    this->modules.append(taskwallfollowing);

    logger->debug("Creating TaskXsensNavigation");
    TaskXsensNavigation *taskxsensnavigation = new TaskXsensNavigation("taskXsensNavi",sim, behavXsens, navi, behavTurn);
    this->modules.append(taskxsensnavigation);

    logger->debug("Creating TaskHandControl");
    TaskHandControl *taskhandcontrol = new TaskHandControl("taskHand", controlLoop, sim, handControl);
    this->modules.append(taskhandcontrol);

    logger->debug("Creating TaskPipeFollowing");
    TaskPipeFollowing *taskpipefollowing = new TaskPipeFollowing("taskPipeFollow", behavPipe, sim, navi, behavTurn);
    this->modules.append(taskpipefollowing);

    logger->debug("Creating TaskMidwaterTarget");
    TaskMidwaterTarget *taskmidwatertarget = new TaskMidwaterTarget("taskMidwater", sim, navi);
    this->modules.append(taskmidwatertarget);


    logger->debug("Creating CommandCenter");
    CommandCenter* commCent = new CommandCenter("comandCenter", controlLoop, handControl, pressure, sim, navi, behavPipe, behavBall, behavTurn, behavWall, behavXsens, taskhandcontrol, taskwallfollowing, taskxsensnavigation, taskpipefollowing, taskmidwatertarget);
    this->modules.append(commCent);

    logger->info("Loading all Modules... Done");

    foreach (RobotModule* b, modules)
    {
        logger->debug("Starting module "+b->getId());
        b->start();
        b->waitForInitToComplete();
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
