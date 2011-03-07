#include "behaviour_wallfollowing.h"
#include <QtGui>
#include <Behaviour_WallFollowing/wallfollowingform.h>
#include <Module_Simulation/module_simulation.h>
#include <Module_ThrusterControlLoop/module_thrustercontrolloop.h>

Behaviour_WallFollowing::Behaviour_WallFollowing(QString id, Module_ThrusterControlLoop* tcl, Module_EchoSounder *echo, Module_Simulation *sim)
    : RobotBehaviour(id)
{
    qDebug() << "wall thread id";
    qDebug() << QThread::currentThreadId();
    this->sim = sim;
    this->tcl = tcl;
    this->echo = echo;
    setDefaultValue("serialPort", "COM9");
    setDefaultValue("range", 5);
    setDefaultValue("forwardSpeed",0.5);
    setDefaultValue("angularSpeed",0.3);
    setDefaultValue("desiredDistance",1.0);

    setEnabled(false);
    QObject::connect(echo,SIGNAL(newWallBehaviourData(const EchoReturnData, float)),this,SLOT(newWallBehaviourData(const EchoReturnData, float)));
    QObject::connect(echo,SIGNAL(dataError()),this,SLOT(stopOnWallError()));
}
bool Behaviour_WallFollowing::isActive()
{
    return isEnabled();
}

void Behaviour_WallFollowing::init()
{
    logger->debug("wall init");
    //timer.moveToThread(this);
    connect(this,SIGNAL(forwardSpeed(float)),tcl,SLOT(setForwardSpeed(float)));
    connect(this,SIGNAL(angularSpeed(float)),tcl,SLOT(setAngularSpeed(float)));

    this->updateFromSettings();
    //connect(&timer,SIGNAL(timeout()),this,SLOT(timerSlot()));
    avgDistance = 0.0;
    distanceInput = 0.0;

}

void Behaviour_WallFollowing::startBehaviour()
{

    this->reset();
    logger->info("Behaviour started" );
    Behaviour_WallFollowing::updateFromSettings();
    this->setHealthToOk();
    this->setEnabled(true);
    //timer.start(timerTime);
    emit started(this);
    //echo->doNextScan();
    running = true;

    qDebug() << "wall thread id";
    qDebug() << QThread::currentThreadId();

}



void Behaviour_WallFollowing::stop()
{
    running = false;
    emit forwardSpeed(0.0);
    emit angularSpeed(0.0);
    // timer.stop();
    if (this->isActive())
    {
        logger->info( "Behaviour stopped" );
        emit forwardSpeed(0.0);
        emit angularSpeed(0.0);
        setEnabled(false);
        emit finished(this,false);
    }
}

void Behaviour_WallFollowing::terminate()
{
    this->stop();
    RobotModule::terminate();
}

void Behaviour_WallFollowing::reset()
{
    emit forwardSpeed(0.0);
    emit angularSpeed(0.0);

}

QList<RobotModule*> Behaviour_WallFollowing::getDependencies()
{
    QList<RobotModule*> ret;
    ret.append(tcl);
    ret.append(echo);
    ret.append(sim);
    return ret;
}

QWidget* Behaviour_WallFollowing::createView(QWidget* parent)
{
    return new WallFollowingForm(parent, this);
}


void Behaviour_WallFollowing::controlWallFollow()
{
    avgDistance = echo->avgDistance;
    float temp = 0.0;

    /**
      * Wie das mit dem Forward und Angular speed geht:
      *
      * range of speed: -1.0 to 1.0
      * positive values = forwards
      * negative values = backwards
      *
      * positive angular speed implies a clockwise rotation.
      * negative angular speed implies a counterclockwise rotation.
      * (both rotations as seem from above the robot)
      *
      * range: -1.0 to 1.0
      */
    if(((avgDistance-0.2) < distanceInput) && (distanceInput < (avgDistance+0.2))){
        wallCase ="Case 1, no turn - only forward";

        emit forwardSpeed(fwdSpeed);
        emit angularSpeed(0.0);
    } else if(avgDistance > distanceInput ){
        wallCase = "Case 2, turn left";

        temp =angSpeed*(-1.0);
        emit forwardSpeed(fwdSpeed);
        emit angularSpeed(temp);

    } else if(avgDistance < distanceInput){
        wallCase = "Case 3, turn right";

        emit forwardSpeed(fwdSpeed);
        emit angularSpeed(angSpeed);
    }
    emit updateWallCase(wallCase);
    addData("Current Case: ",wallCase);
    emit dataChanged(this);

}



void Behaviour_WallFollowing::newWallBehaviourData(const EchoReturnData data, float avgDistance)
{
    if(isEnabled()){
        emit newWallUiData(data, avgDistance);
        this->avgDistance = avgDistance;

        if(sim->isEnabled())
        {
            //
        } else {

            if(avgDistance > 0.0)
            {
                if(isEnabled() && this->getHealthStatus().isHealthOk()){
                    Behaviour_WallFollowing::controlWallFollow();
                }
            } else {
                this->setHealthToSick("average distance missing");
                emit forwardSpeed(0.0);
                emit angularSpeed(0.0);
                wallCase = "Case 4: No average distance, stop thruster!";
                emit updateWallCase(wallCase);
            }
        }
    }
}


void Behaviour_WallFollowing::updateFromSettings()
{
    this->distanceInput = this->getSettingsValue("desiredDistance").toFloat();
    this->fwdSpeed = this->getSettingsValue("forwardSpeed").toFloat();
    this->angSpeed = this->getSettingsValue("angularSpeed").toFloat();
}

void Behaviour_WallFollowing::stopOnWallError(){
    emit forwardSpeed(0.0);
    emit angularSpeed(0.0);
    wallCase = "Case 4: No data/source, stop thruster!";
    emit updateWallCase(wallCase);
    addData("Current Case: ",wallCase);
    emit dataChanged(this);
}
