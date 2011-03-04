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

    setEnabled(false);
    QObject::connect(echo,SIGNAL(newWallBehaviourData(const EchoReturnData, float)),this,SLOT(newWallBehaviourData(const EchoReturnData, float)));
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
    QString fall="";
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
        qDebug("Fall1: avgDistance == distanceInput");
           fall ="Fall 1";
        addData("Fall1: angSpeed", angSpeed);
        addData("Fall1: fwdSpeed",fwdSpeed);
        emit dataChanged(this);

        emit forwardSpeed(fwdSpeed);
        emit angularSpeed(0.0);
    } else if(avgDistance > distanceInput ){
fall = "Fall 2";
        addData("Fall2: angSpeed", angSpeed);
        addData("Fall2: fwdSpeed",fwdSpeed);
        emit dataChanged(this);
        temp =angSpeed*(-1.0);
            emit forwardSpeed(fwdSpeed);
            emit angularSpeed(temp);

    } else if(avgDistance < distanceInput){
     //addData("Fall3: avgDistance < distanceInput");
     addData("Fall3: angSpeed", angSpeed);
     addData("Fall3: fwdSpeed",fwdSpeed);
     fall = "Fall 3";
     emit dataChanged(this);
            emit forwardSpeed(fwdSpeed);
            emit angularSpeed(angSpeed);
    }
    addData("Verhalten:",fall);
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
        }
    }
}
}


void Behaviour_WallFollowing::updateFromSettings()
{
    this->distanceInput = this->getSettingsValue("distanceInput").toFloat();
    this->fwdSpeed = this->getSettingsValue("forwardInput").toFloat();
    this->angSpeed = this->getSettingsValue("angularInput").toFloat();
}

void Behaviour_WallFollowing::stopOnWallError(){

}

