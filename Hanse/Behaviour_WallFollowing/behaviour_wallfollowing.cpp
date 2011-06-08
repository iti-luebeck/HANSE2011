#include "behaviour_wallfollowing.h"
#include <QtGui>
#include <Behaviour_WallFollowing/wallfollowingform.h>
#include <Module_Simulation/module_simulation.h>
#include <Module_ThrusterControlLoop/module_thrustercontrolloop.h>
#include <Module_XsensMTi/module_xsensmti.h>
#include <Framework/Angles.h>

Behaviour_WallFollowing::Behaviour_WallFollowing(QString id, Module_ThrusterControlLoop* tcl, Module_EchoSounder *echo, Module_XsensMTi* x)
    : RobotBehaviour(id)
{
    this->tcl = tcl;
    this->echo = echo;
    this->xsens = x;

    setDefaultValue("serialPort", "COM9");
    setDefaultValue("range", 5);
    setDefaultValue("forwardSpeed",0.5);
    setDefaultValue("angularSpeed",0.3);
    setDefaultValue("desiredDistance",1.5);
    setDefaultValue("corridorWidth",0.2);

    setDefaultValue("wallTime",2000);
    setDefaultValue("initHeading", 0);

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
    distanceInput = this->getSettingsValue("desiredDistance").toFloat();

    echoControlTimer = new QTimer(this);
    connect(echoControlTimer, SIGNAL(timeout()), this, SLOT(testEchoModule()));

    connect(this, SIGNAL(enabled(bool)), this, SLOT(controlEnabledChanged(bool)));

}

void Behaviour_WallFollowing::startBehaviour()
{
    if (this->isEnabled() == true){
        logger->info("Already enabled/started!");
        return;
    }

    this->echo->setEnabled(true);
    this->reset();
    logger->info("Behaviour started" );
    Behaviour_WallFollowing::updateFromSettings();
    this->setHealthToOk();
    this->setEnabled(true);
    emit started(this);
    running = true;
    if(this->getSettingsValue("useInitHeading").toBool() == false){
        logger->info("No init heading used");
        initHeadingReached = true;
    } else {
        initHeadingReached = false;
        wallCase = "Adjust initial heading...";
        emit updateWallCase(wallCase);
        addData("Current Case: ",wallCase);
        emit dataChanged(this);
        QTimer::singleShot(0, this, SLOT(controlInitHeading()));
    }

    echoControlTimer->start(wallTime);
}


void Behaviour_WallFollowing::stop()
{
    if (this->isEnabled() == false){
        logger->info("Not enabled!");
        return;
    }

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
        echoControlTimer->stop();
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
    return ret;
}

QWidget* Behaviour_WallFollowing::createView(QWidget* parent)
{
    return new WallFollowingForm(parent, this);
}


void Behaviour_WallFollowing::controlWallFollow()
{
    if (this->isEnabled() == false){
        logger->info("Not enabled!");
        return;
    }

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
            }
        } else if(avgDistance > distanceInput ){
            if(wallCase!="Case 3: Turn left"){
                wallCase = "Case 3: Turn left";
                temp =angSpeed*(-1.0);
                emit forwardSpeed(fwdSpeed);
                emit angularSpeed(temp);
            }
        } else if(avgDistance < distanceInput){   
            if(wallCase!="Case 2: Turn right"){
                wallCase = "Case 2: Turn right";
                emit forwardSpeed(fwdSpeed);
                emit angularSpeed(angSpeed);
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
        if(this->initHeadingReached == true){
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
            wallCase = "Adjust initial heading...";
            emit updateWallCase(wallCase);
            addData("Current Case: ",wallCase);
            emit dataChanged(this);
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
    this->wallTime = this->getSettingsValue("wallTime").toInt();
    updateUi();
}

void Behaviour_WallFollowing::stopOnEchoError(){
    if (this->isEnabled() == false){
        logger->info("Not enabled!");
        return;
    }
    emit forwardSpeed(0.0);
    emit angularSpeed(0.0);
    wallCase = "No echosignal, stop thruster!";
    emit updateWallCase(wallCase);
    addData("Current Case: ",wallCase);
    emit dataChanged(this);
}

void Behaviour_WallFollowing::testEchoModule(){
    if(this->isEnabled()){
        if(!echo->isEnabled()){
            emit dataError();
        }
    }
    wallCase = "";
}

void Behaviour_WallFollowing::controlEnabledChanged(bool b){
    if(b == false){
        logger->info("No longer enabled!");
        QTimer::singleShot(0, this, SLOT(stop()));
    }
}

void Behaviour_WallFollowing::controlInitHeading(){
    if(this->isEnabled()){
        logger->info("Ctrl init heading");
        if(this->xsens->isEnabled()){
            float currentHeading = this->xsens->getHeading();
            float targetHeading = this->getSettingsValue("initHeading").toFloat();
            float diffHeading = Angles::deg2deg(targetHeading - currentHeading);
            addData("currentHeading", currentHeading);
            addData("targetHeading", targetHeading);
            addData("diffHeading", diffHeading);
            emit dataChanged(this);
            if(-10 < diffHeading && diffHeading < 10){
                logger->info("init heading reached");
                initHeadingReached = true;
            } else {
                initHeadingReached = false;
                float ctrAngleSpeed = 0.0;
                if(diffHeading > 10 || diffHeading < 10)
                {
                    diffHeading /= this->getSettingsValue("targetHeading").toDouble();
                    ctrAngleSpeed = 0.4 * diffHeading;
                }
                emit angularSpeed(ctrAngleSpeed);
                QTimer::singleShot(100, this, SLOT(controlInitHeading()));
            }
        } else {
            logger->info("Sry, xsens not enabled - start wallfollowing without initial heading");
            initHeadingReached = true;
        }
    }
}
