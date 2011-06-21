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
    setDefaultValue("p", 0.3);
    setDefaultValue("useP", false);
    setDefaultValue("experimentalMode", false);

    setEnabled(false);
    QObject::connect(echo,SIGNAL(newWallBehaviourData(const EchoReturnData, float)),this,SLOT(newWallBehaviourData(const EchoReturnData, float)));
    QObject::connect(echo,SIGNAL(dataError()),this,SLOT(stopOnEchoError()));
    QObject::connect(this,SIGNAL(dataError()),this,SLOT(stopOnEchoError()));

    running = false;
    t90dt90 = false;
    startPhase = true;
    diff = 0.0;

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
    if (!this->isEnabled()){
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
    t90dt90 = false;
    startPhase = true;

    echoControlTimer->start(wallTime);
}


void Behaviour_WallFollowing::stop()
{
    if (!this->isEnabled()){
        logger->info("Not enabled!");
        return;
    }

    running = false;
    t90dt90 = false;
    startPhase = false;
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
    if(this->isEnabled()){
        emit forwardSpeed(0.0);
        emit angularSpeed(0.0);
        stop();
        startBehaviour();
    }
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
    if (!this->isEnabled()){
        logger->info("Not enabled!");
        return;
    }

    avgDistance = echo->avgDistance;

    addData("Desired distance: ",distanceInput);
    addData("Avg distance: ",avgDistance);
    emit dataChanged(this);

    if(running){
        if(!this->t90dt90){
            diff = distanceInput - avgDistance;
            if(this->getSettingsValue("experimentalMode").toBool()){
                QTimer::singleShot(0, this, SLOT(controlWallFollowThruster()));
            } else {
                // Experimental mode
                if((fabs(diff) < 0.8)){
                    QTimer::singleShot(0, this, SLOT(controlWallFollowThruster()));
                } else if(!startPhase){
                    QTimer::singleShot(0, this, SLOT(controlWallFollowThruster()));
                } else {
                    initialHeading = this->xsens->getHeading();
                    addData("initialHeading",initialHeading);
                    this->t90dt90 = true;
                    wallCase = "Turn 90; Drive; Turn 90 back";
                    emit updateWallCase(wallCase);
                    addData("Current Case: ",wallCase);
                    emit dataChanged(this);
                    QTimer::singleShot(0, this, SLOT(turn90One()));
                }
            }
        }
    }
    emit updateWallCase(wallCase);
    addData("Current Case: ",wallCase);
    emit dataChanged(this);
}

void Behaviour_WallFollowing::controlWallFollowThruster(){
    if(this->isEnabled()){
        // Now the disired distance is reached, start phase finished.
        startPhase = false;
        if(this->getSettingsValue("useP").toBool()){
            float ctrAngle = 0.0;
            wallCase ="P Control: Turn";
            ctrAngle = this->getSettingsValue("p").toFloat()*diff;
            emit forwardSpeed(fwdSpeed);
            emit angularSpeed(ctrAngle);
        } else {
            float temp = 1.0;
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
    }
}


void Behaviour_WallFollowing::newWallBehaviourData(const EchoReturnData data, float avgDistance)
{
    if(this->isActive() && this->running){
        if(echo->isEnabled()){
            emit newWallUiData(data, avgDistance);
            this->avgDistance = avgDistance;
            if(!t90dt90){
                if((avgDistance > 0.0) && (this->getHealthStatus().isHealthOk())){
                    Behaviour_WallFollowing::controlWallFollow();
                } else if((avgDistance == 0.0) && (this->getHealthStatus().isHealthOk())){
                    if(wallCase!="No average distance (no wall)?! Only turn left..."){
                        emit forwardSpeed(0.0);
                        emit angularSpeed(-angSpeed);
                        wallCase = "No average distance (no wall)?! Only turn left...";
                        emit updateWallCase(wallCase);
                        addData("Current Case: ",wallCase);
                        emit dataChanged(this);
                    }
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
    this->wallTime = this->getSettingsValue("wallTime").toInt();
    updateUi();
}

void Behaviour_WallFollowing::stopOnEchoError(){
    if (!this->isEnabled()){
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

void Behaviour_WallFollowing::turn90One(){
    if(this->isEnabled()){

        double currentHeading = 0.0;
        currentHeading = this->xsens->getHeading();
        double targetHeading;
        if(diff < 0){
            targetHeading = Angles::deg2deg(initialHeading - 90);
        } else {
            targetHeading = Angles::deg2deg(initialHeading + 90);
        }


        double diffHeading = Angles::deg2deg(targetHeading - currentHeading);
        addData("initialHeading",initialHeading);
        addData("targetHeading",targetHeading);
        addData("diffHeading",diffHeading);
        emit dataChanged(this);
        if (fabs(diffHeading) < 10)
        {
            logger->info("Turn90OneÁdjust heading finished");
            emit angularSpeed(0.0);
            emit forwardSpeed(0.0);
            QTimer::singleShot(0, this, SLOT(drive()));
        }
        else
        {
            logger->info("Turn90OneÁdjust heading");
            qDebug()<<"diffHeading"<<diffHeading;
            double angularSpeedValue = 0.2 * diffHeading;
            emit angularSpeed(angularSpeedValue);
            emit forwardSpeed(0.0);
            QTimer::singleShot(100, this, SLOT(turn90One()));
        }
    }
}

void Behaviour_WallFollowing::drive(){
    if(this->isEnabled()){
        logger->info("Turn90 drive");
        emit angularSpeed(0.0);
        emit forwardSpeed(1.0);
        initialHeading = this->xsens->getHeading();
        QTimer::singleShot(3000, this, SLOT(turn90Two()));
    }
}

void Behaviour_WallFollowing::turn90Two(){
    if(this->isEnabled()){
        double currentHeading = 0.0;
        currentHeading = this->xsens->getHeading();
        double targetHeading;
        if(diff < 0){
            targetHeading = Angles::deg2deg(initialHeading + 90);
        } else {
            targetHeading = Angles::deg2deg(initialHeading - 90);
        }
        double diffHeading = Angles::deg2deg(targetHeading - currentHeading);
        addData("initialHeading",initialHeading);
        addData("targetHeading",targetHeading);
        addData("diffHeading",diffHeading);
        emit dataChanged(this);
        if (fabs(diffHeading) < 10)
        {

            emit angularSpeed(0.0);
            emit forwardSpeed(0.0);
            this->t90dt90 = false;
            qDebug("Turn90DriveTurn90 finished!");

        }
        else
        {
            logger->info("Turn90OneÁdjust heading");
            qDebug()<<"diffHeading"<<diffHeading;
            double angularSpeedValue = 0.2 * diffHeading;
            emit angularSpeed(angularSpeedValue);
            QTimer::singleShot(100, this, SLOT(turn90Two()));
        }
    }
}
