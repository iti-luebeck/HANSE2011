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

}
bool Behaviour_WallFollowing::isActive()
{
    return active;
}

void Behaviour_WallFollowing::init()
{
    active = false;
    setEnabled(false);
    logger->debug("wall init");

    setDefaultValue("serialPort", "COM9");
    setDefaultValue("range", 5);
    setDefaultValue("forwardSpeed",0.5);
    setDefaultValue("angularSpeed",0.3);
    setDefaultValue("desiredDistance",1.5);
    setDefaultValue("corridorWidth",0.2);

    setDefaultValue("wallTime",2000);
    setDefaultValue("p", 0.3);
    setDefaultValue("useP", false);

    setDefaultValue("allowedDist", 0.8);
    setDefaultValue("useStartAdjust", true);


    QObject::connect(echo,SIGNAL(newWallBehaviourData(const EchoReturnData, float)),this,SLOT(newData(const EchoReturnData, float)));
    QObject::connect(echo,SIGNAL(dataError()),this,SLOT(stopOnEchoError()));
    QObject::connect(this,SIGNAL(dataError()),this,SLOT(stopOnEchoError()));
    connect(this,SIGNAL(forwardSpeed(float)),tcl,SLOT(setForwardSpeed(float)));
    connect(this,SIGNAL(angularSpeed(float)),tcl,SLOT(setAngularSpeed(float)));

    avgDistance = 0.0;
    distanceInput = 0.0;
    diff = 0.0;

    connect(this, SIGNAL(enabled(bool)), this, SLOT(controlEnabledChanged(bool)));
}

void Behaviour_WallFollowing::startBehaviour()
{
    if (isActive()){
        logger->info("Already active!");
        return;
    }

    reset();

    logger->info(" - Behaviour started -");
    wallState = WALL_ADJUST_START;
    adjustState = ADJUST_IDLE;
    updateOutput();

    updateFromSettings();
    echo->setEnabled(true);

    setHealthToOk();
    emit started(this);

    if(!echo->isEnabled()){
        emit dataError();
    }

    active = true;
    if(!isEnabled()){
        setEnabled(true);
    }

}


void Behaviour_WallFollowing::stop()
{
    if(!isActive()){
        logger->info("Not active!");
        return;
    }

    active = false;
    setEnabled(false);


    wallState = WALL_STOP;
    adjustState = ADJUST_IDLE;
    updateOutput();

    emit forwardSpeed(0.0);
    emit angularSpeed(0.0);


    emit finished(this,false);

}

void Behaviour_WallFollowing::terminate()
{
    stop();
    RobotModule::terminate();
}

void Behaviour_WallFollowing::reset()
{
    emit forwardSpeed(0.0);
    emit angularSpeed(0.0);
    avgDistance = 0.0;
    diff = 0.0;
    wallState = WALL_ADJUST_START;
    adjustState = ADJUST_IDLE;
}



void Behaviour_WallFollowing::newData(const EchoReturnData data, float avgDist)
{
    if(!isActive()){
        return;
    }

    emit newWallUiData(data, avgDist);
    avgDistance = avgDist;
    addData("Distance average",avgDistance);
    emit dataChanged(this);

    if(avgDistance > 0.0){
        controlWallFollow(); 
    } else if(adjustState == ADJUST_IDLE){
        emit forwardSpeed(0.0);
        if(FLAG_SOUNDER_RIGHT){
            // Turn right
            emit angularSpeed(angSpeed);
        } else {
            // Turn left
            emit angularSpeed(-angSpeed);
        }
        wallState = WALL_NO_WALL;
        emit updateGUI(WALL_NO_WALL);
        addData("wall",WALL_NO_WALL);
        emit dataChanged(this);
    }
}



void Behaviour_WallFollowing::controlWallFollow()
{
    if (!isActive()){
        return;
    }

    addData("Distance target",distanceInput);
    addData("Distance average",avgDistance);
    emit dataChanged(this);

    diff = distanceInput - avgDistance;

    if(wallState == WALL_ADJUST_START){
        if(getSettingsValue("useStartAdjust").toBool()){
            if(adjustState == ADJUST_IDLE){
                if((fabs(diff) < allowedDist)){
                    wallState = WALL_CONTROL_WALLFOLLOW;
                    updateOutput();
                } else if(adjustState == ADJUST_IDLE){
                    initialHeading = this->xsens->getHeading();
                    adjustTurnOne();
                }
            }
        } else {
            wallState = WALL_CONTROL_WALLFOLLOW;
            updateOutput();
        }
    } else {
        wallState = WALL_CONTROL_WALLFOLLOW;
        if(FLAG_SOUNDER_RIGHT){
            controlThrusterEchoRight();
        } else {
            controlThrusterEchoLeft();
        }

    }
}


void Behaviour_WallFollowing::controlThrusterEchoRight(){
    if(!isActive()){
        return;
    }

    if(this->getSettingsValue("useP").toBool()){

        float ctrAngle = 0.0;
        if(diff < 0){
            wallState = WALL_TURN_RIGHT;
            emit updateGUI(wallState);
        } else {
            wallState = WALL_TURN_LEFT;
            emit updateGUI(wallState);
        }
        ctrAngle = this->getSettingsValue("p").toFloat()*diff*(-1);
        emit forwardSpeed(fwdSpeed);
        emit angularSpeed(ctrAngle);

    } else {

        float temp = 1.0;
        if(((avgDistance-corridorWidth) < distanceInput) && (distanceInput < (avgDistance+corridorWidth))){

            wallState = WALL_NO_TURN;
            emit updateGUI(wallState);
            emit forwardSpeed(fwdSpeed);
            emit angularSpeed(0.0);

        } else if(avgDistance < distanceInput ){

            wallState = WALL_TURN_LEFT;
            emit updateGUI(wallState);
            temp =angSpeed*(-1.0);
            emit forwardSpeed(fwdSpeed);
            emit angularSpeed(temp);

        } else if(avgDistance > distanceInput){

            wallState = WALL_TURN_RIGHT ;
            emit updateGUI(wallState);
            emit forwardSpeed(fwdSpeed);
            emit angularSpeed(angSpeed);

        }
    }
}



void Behaviour_WallFollowing::controlThrusterEchoLeft(){
    if(!isActive()){
        return;
    }

    if(this->getSettingsValue("useP").toBool()){

        float ctrAngle = 0.0;
        if(diff < 0){
            wallState = WALL_TURN_LEFT;
            emit updateGUI(wallState);
        } else {
            wallState = WALL_TURN_RIGHT;
            emit updateGUI(wallState);
        }
        ctrAngle = this->getSettingsValue("p").toFloat()*diff*(-1);
        emit forwardSpeed(fwdSpeed);
        emit angularSpeed(ctrAngle);

    } else {

        float temp = 1.0;
        if(((avgDistance-corridorWidth) < distanceInput) && (distanceInput < (avgDistance+corridorWidth))){

            wallState = WALL_NO_TURN;
            emit updateGUI(wallState);
            emit forwardSpeed(fwdSpeed);
            emit angularSpeed(0.0);

        } else if(avgDistance > distanceInput ){

            wallState = WALL_TURN_LEFT;
            emit updateGUI(wallState);
            temp =angSpeed*(-1.0);
            emit forwardSpeed(fwdSpeed);
            emit angularSpeed(temp);

        } else if(avgDistance < distanceInput){

            wallState = WALL_TURN_RIGHT;
            emit updateGUI(wallState);
            emit forwardSpeed(fwdSpeed);
            emit angularSpeed(angSpeed);

        }
    }
}


void Behaviour_WallFollowing::adjustTurnOne(){
    if(!isActive()){
        return;
    }

    if(adjustState != ADJUST_TURN90_START){
        adjustState = ADJUST_TURN90_START;
        guiShow = adjustState;
        updateOutput();

        if(FLAG_SOUNDER_RIGHT == false){
            if(diff < 0){
                targetHeading = Angles::deg2deg(initialHeading - 90);
            } else {
                targetHeading = Angles::deg2deg(initialHeading + 90);
            }
        } else {
            if(diff < 0){
                targetHeading = Angles::deg2deg(initialHeading + 90);
            } else {
                targetHeading = Angles::deg2deg(initialHeading - 90);
            }
        }
    }

    float currentHeading = this->xsens->getHeading();
    float diffHeading = Angles::deg2deg(targetHeading - currentHeading);
    addData("heading initial", initialHeading);
    addData("heading target", targetHeading);
    addData("heading difference", diffHeading);
    emit dataChanged(this);

    if (fabs(diffHeading) < 10){

        emit angularSpeed(0.0);
        emit forwardSpeed(0.0);
        QTimer::singleShot(0, this, SLOT(adjustDrive()));

    } else {

        double angularSpeedValue = 0.2 * diffHeading;
        emit angularSpeed(angularSpeedValue);
        emit forwardSpeed(0.0);
        QTimer::singleShot(100, this, SLOT(adjustTurnOne()));

    }

}

void Behaviour_WallFollowing::adjustDrive(){
    if(!isActive()){
        return;
    }

    adjustState = ADJUST_DRIVE;
    guiShow = adjustState;
    updateOutput();

    emit angularSpeed(0.0);
    emit forwardSpeed(1.0);
    int driveTime = this->getSettingsValue("driveTime").toInt();
    QTimer::singleShot(driveTime, this, SLOT(adjustTurnTwo()));

}

void Behaviour_WallFollowing::adjustTurnTwo(){
    if(!isActive()){
        return;
    }

    if(adjustState != ADJUST_TURN90_END){
        adjustState = ADJUST_TURN90_END;
        guiShow = adjustState;
        updateOutput();

        targetHeading = initialHeading;
    }

    float currentHeading = this->xsens->getHeading();
    float diffHeading = Angles::deg2deg(targetHeading - currentHeading);
    addData("heading target", targetHeading);
    addData("heading difference", diffHeading);
    emit dataChanged(this);

    if (fabs(diffHeading) < 10){

        emit angularSpeed(0.0);
        emit forwardSpeed(0.0);
        adjustState = ADJUST_IDLE;
        guiShow = "";
        updateOutput();
    } else {

        double angularSpeedValue = 0.2 * diffHeading;
        emit angularSpeed(angularSpeedValue);
        emit forwardSpeed(0.0);
        QTimer::singleShot(100, this, SLOT(adjustTurnTwo()));

    }

}


void Behaviour_WallFollowing::updateFromSettings()
{
    updateUi();
    this->allowedDist = this->getSettingsValue("allowedDist").toFloat();
    this->distanceInput = this->getSettingsValue("desiredDistance").toFloat();
    this->fwdSpeed = this->getSettingsValue("forwardSpeed").toFloat();
    this->angSpeed = this->getSettingsValue("angularSpeed").toFloat();
    this->corridorWidth = this->getSettingsValue("corridorWidth").toFloat();

}

void Behaviour_WallFollowing::stopOnEchoError(){
    if(!isActive()){
        return;
    }

    emit forwardSpeed(0.0);
    emit angularSpeed(0.0);
    wallState = WALL_ERROR;
    emit updateGUI(wallState);
    addData("wallState",wallState);
    emit dataChanged(this);
}

void Behaviour_WallFollowing::updateOutput(){

    addData("wallState",wallState);
    addData("adjustState", adjustState);
    emit dataChanged(this);
    if(guiShow != ""){
        emit updateGUI(guiShow);
        logger->info(guiShow);
    } else {
        emit updateGUI(wallState);
        logger->info(wallState);
    }

}

void Behaviour_WallFollowing::controlEnabledChanged(bool enabled){
    if(!enabled && isActive()){
        logger->info("Disable and deactivate WallFollowing");
        stop();
    } else if(!enabled && !isActive()){
        logger->info("Still deactivated");
    } else if(enabled && !isActive()){
        logger->info("Enable and activate WallFollowing");
        startBehaviour();
    } else {
        logger->info("Still activated");
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

