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
//#include <Module_VisualSLAM/module_visualslam.h>
#include <Module_Navigation/module_navigation.h>
#include <Behaviour_PipeFollowing/behaviour_pipefollowing.h>
#include <Behaviour_GoalFollowing/behaviour_goalfollowing.h>
#include <Behaviour_BallFollowing/behaviour_ballfollowing.h>
#include <Module_Webcams/module_webcams.h>
#include <MetaBehaviour/metabehaviour.h>
#include <Behaviour_TurnOneEighty/behaviour_turnoneeighty.h>
#include <Behaviour_CompassFollowing/behaviour_compassfollowing.h>
#include <Module_ADC/module_adc.h>
#include <Module_Simulation/module_simulation.h>
#include <Module_EchoSounder/module_echosounder.h>
#include <Module_XsensMTi/module_xsensmti.h>
#include <Behaviour_WallFollowing/behaviour_wallfollowing.h>
#include <CommandCenter/commandcenter.h>
#include <TestTask/testtask.h>
#include <TaskWallFollowing/taskwallfollowing.h>
#include "SoToSleep.h"
#include <TaskThrusterControl/taskthrustercontrol.h>

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

    Module_IMU* imu = new Module_IMU("adis",uid,sim);
    this->modules.append(imu);

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
    Module_ScanningSonar* sonar = new Module_ScanningSonar("sonar",controlLoop,sim);
    this->modules.append(sonar);

    logger->debug("Creating Module_EchoSounder");
    Module_EchoSounder* echo = new Module_EchoSounder("echo",sim);
    this->modules.append(echo);


    logger->debug("Creating Module_HandControl");
    Module_HandControl* handControl = new Module_HandControl("handControl",controlLoop, thrusterLeft, thrusterRight, thrusterDown, thrusterDownF);
    this->modules.append(handControl);

    logger->debug("Creating Module_SonarLocalization");
    Module_SonarLocalization* sonarLoc = new Module_SonarLocalization("sonarLocalize", sonar, xsens, sim);
    this->modules.append(sonarLoc);

    logger->debug("Creating Module_Webcams");
    Module_Webcams *cams = new Module_Webcams( "cams" );
    this->modules.append( cams );

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
    Behaviour_TurnOneEighty* behavTurn = new Behaviour_TurnOneEighty("turn",controlLoop, compass);
    this->modules.append(behavTurn);

    logger->debug("Creating Behaviour_WallFollowing");
    Behaviour_WallFollowing* behavWall = new Behaviour_WallFollowing("wall",controlLoop, echo, sim);
    this->modules.append(behavWall);

    //    logger->debug("Creating Behaviour_CompassFollowing");
    //    Behaviour_CompassFollowing* behavComp = new Behaviour_CompassFollowing("compFollow",controlLoop, compass);
    //    this->modules.append(behavComp);

    logger->debug("Creating TestTask");
    TestTask *testtask = new TestTask("testtask", behavWall, sim);
    //this->modules.append(testtask);

    logger->debug("Creating TaskWallFollowing");
    TaskWallFollowing *taskwallfollowing = new TaskWallFollowing("taskWallFo", behavWall, sim);
    this->modules.append(taskwallfollowing);

    logger->debug("Creating TaskThrusterControl");
    TaskThrusterControl *taskthrustercontrol = new TaskThrusterControl("taskThrustCo", controlLoop, sim);
    this->modules.append(taskthrustercontrol);

    logger->debug("Creating CommandCenter");
    CommandCenter* commCent = new CommandCenter("comCent", controlLoop, handControl, pressure, sim, testtask, taskwallfollowing, taskthrustercontrol);
    this->modules.append(commCent);

    // IMPORTANT: must be the last module to be loaded, otherwise it won't have access to all the other modules
    logger->debug("Creating MetaBehaviour");
    MetaBehaviour* metaBehaviour = new MetaBehaviour("meta",this, controlLoop, handControl, pressure, behavPipe, behavBall, behavTurn, behavWall, commCent);
    this->modules.append(metaBehaviour);


    logger->info("Loading all Modules... Done");

    /* connect every modul to healtCheckTimer */
    //    connect(&healthTimer,SIGNAL(timeout()),controlLoop,SLOT(doHealthCheck()));
    foreach (RobotModule* b, modules)
    {
        connect(&healthTimer,SIGNAL(timeout()),b,SLOT(doHealthCheck()));
        //        RobotBehaviour* c = dynamic_cast<RobotBehaviour*>(b);
        //        if (!c || (c->getId() == "meta"))
        //        {
        logger->debug("Starting Thread for "+b->getId());
        b->start();
        while(!b->isInitialized())
            sotoSleep::msleep(100);

        //        }
    }
    healthTimer.setInterval(1000);
    //    QTimer::singleShot(0,&healthTimer,SLOT(start()));
    healthTimer.start(1000);

    // Few manual connects/starts for the Commandcenter-Tasks which have no GUI
    connect(&healthTimer,SIGNAL(timeout()),testtask,SLOT(doHealthCheck()));
    logger->debug("Starting Thread for "+testtask->getId());
    testtask->start();

}

QList<RobotModule*> ModulesGraph::getModules()
{
    return QList<RobotModule*>(modules);
}

void ModulesGraph::HastaLaVista()
{
    logger->info("Terminating all modules...");
    // go backwards through the module list
    healthTimer.stop();
    for (int i = modules.size()-1; i>=0; i--) {
        logger->info("Terminating "+modules[i]->getId());
        modules[i]->shutdown();
    }
    logger->info("All modules terminated.");
}
