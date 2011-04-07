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
    setDefaultValue("corridorWidth",0.2);

    setEnabled(false);
    QObject::connect(echo,SIGNAL(newWallBehaviourData(const EchoReturnData, float)),this,SLOT(newWallBehaviourData(const EchoReturnData, float)));
    QObject::connect(echo,SIGNAL(dataError()),this,SLOT(stopOnEchoError()));
    QObject::connect(this,SIGNAL(dataError()),this,SLOT(stopOnEchoError()));

    running = false;

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
    avgDistance = 1.0;
    distanceInput = 1.0;

    echoControllTimer = new QTimer(this);
    connect(echoControllTimer, SIGNAL(timeout()), this, SLOT(testEchoModule()));

}

void Behaviour_WallFollowing::startBehaviour()
{
    this->reset();
    logger->info("Behaviour started" );
    Behaviour_WallFollowing::updateFromSettings();
    this->setHealthToOk();
    this->setEnabled(true);
    emit started(this);
    running = true;
    echoControllTimer->start(1000);

    //qDebug() << "wall thread id";
    //qDebug() << QThread::currentThreadId();
}


void Behaviour_WallFollowing::stop()
{
    running = false;
    emit forwardSpeed(0.0);
    emit angularSpeed(0.0);

    if (this->isActive())
    {
        logger->info( "Behaviour stopped" );
        wallCase = "Wallfollowing stopped, stop thruster";
        emit updateWallCase(wallCase);
        addData("Current Case: ",wallCase);
        emit dataChanged(this);
        emit forwardSpeed(0.0);
        emit angularSpeed(0.0);
        this->setEnabled(false);
        emit finished(this,false);

        echoControllTimer->stop();
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
    addData("Desired distance: ",distanceInput);
    addData("Avg distance: ",avgDistance);
    emit dataChanged(this);
    tempAs ="";
    if(running==true){
        if(((avgDistance-corridorWidth) < distanceInput) && (distanceInput < (avgDistance+corridorWidth))){          
            if(wallCase!="Case 1: No turn - only forward"){
                wallCase ="Case 1: No turn - only forward";
                emit forwardSpeed(fwdSpeed);
                emit angularSpeed(0.0);
                //qDebug("Case1");
            }
        } else if(avgDistance > distanceInput ){
            if(wallCase!="Case 3: Turn left"){
                wallCase = "Case 3: Turn left";
                temp =angSpeed*(-1.0);
                emit forwardSpeed(fwdSpeed);
                emit angularSpeed(temp);
                //qDebug("Case3");
            }
        } else if(avgDistance < distanceInput){   
            if(wallCase!="Case 2: Turn right"){
                wallCase = "Case 2: Turn right";
                emit forwardSpeed(fwdSpeed);
                emit angularSpeed(angSpeed);
                //qDebug("Case2");
            }
        }
    }
    emit updateWallCase(wallCase);
    addData("Current Case: ",wallCase);
    emit dataChanged(this);
}



void Behaviour_WallFollowing::newWallBehaviourData(const EchoReturnData data, float avgDistance)
{
    if(this->isActive() && this->running == true){
        if(echo->isEnabled()){
            emit newWallUiData(data, avgDistance);
            this->avgDistance = avgDistance;
            if((avgDistance > 0.0) && (this->getHealthStatus().isHealthOk())){
                Behaviour_WallFollowing::controlWallFollow();
            } else if((avgDistance == 0.0) && (this->getHealthStatus().isHealthOk())){
                if(wallCase!="Case 4: No average distance (no wall)?! Only turn right..."){
                    emit forwardSpeed(0.0);
                    emit angularSpeed(angSpeed);
                    wallCase = "Case 4: No average distance (no wall)?! Only turn right...";
                    emit updateWallCase(wallCase);
                    addData("Current Case: ",wallCase);
                    emit dataChanged(this);
                }
            }
        } else if(!echo->isEnabled()){
            emit dataError();
        } else {
            this->setHealthToSick("Something is really wrong, stop thruster");
            if(wallCase!="Something is really wrong, stop thruster"){
                emit forwardSpeed(0.0);
                emit angularSpeed(0.0);
                wallCase = "Something is really wrong, stop thruster";
                emit updateWallCase(wallCase);
                addData("Current Case: ",wallCase);
                emit dataChanged(this);
            }
        }
    } else {
        if(wallCase!="Wallfollowing not activated!"){
            wallCase = "Wallfollowing not activated!";
            emit updateWallCase(wallCase);
            addData("Current Case: ",wallCase);
            emit dataChanged(this);
        }
    }
}


void Behaviour_WallFollowing::updateFromSettings()
{
    this->distanceInput = this->getSettingsValue("desiredDistance").toFloat();
    this->fwdSpeed = this->getSettingsValue("forwardSpeed").toFloat();
    this->angSpeed = this->getSettingsValue("angularSpeed").toFloat();
    this->corridorWidth = this->getSettingsValue("corridorWidth").toFloat();
}

void Behaviour_WallFollowing::stopOnEchoError(){
    emit forwardSpeed(0.0);
    emit angularSpeed(0.0);
    wallCase = "No echosignal, stop thruster!";
    emit updateWallCase(wallCase);
    addData("Current Case: ",wallCase);
    emit dataChanged(this);
}

void Behaviour_WallFollowing::testEchoModule(){
    //qDebug("Test");
    if(this->isEnabled()){
        if(!echo->isEnabled()){
            emit dataError();
        }
    }

}
