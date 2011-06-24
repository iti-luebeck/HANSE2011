#include "behaviour_turnoneeighty.h"
#include <Module_ThrusterControlLoop/module_thrustercontrolloop.h>
#include <Behaviour_TurnOneEighty/form_turnoneeighty.h>
#include <Module_XsensMTi/module_xsensmti.h>
#include <Framework/Angles.h>

Behaviour_TurnOneEighty::Behaviour_TurnOneEighty( QString id, Module_ThrusterControlLoop* tcl, Module_XsensMTi *x) :
    RobotBehaviour( id )
{
    this->tcl = tcl;
    this->xsens = x;   
}

void Behaviour_TurnOneEighty::init()
{
    active = false;
    QObject::connect( xsens, SIGNAL( dataChanged(RobotModule*) ),
                      this, SLOT( xsensUpdate(RobotModule*) ) );
    connect(this,SIGNAL(setAngularSpeed(float)),tcl,SLOT(setAngularSpeed(float)));
    connect(this, SIGNAL(enabled(bool)), this, SLOT(controlEnabledChanged(bool)));
    this->setDefaultValue("degree", 180);
}

QList<RobotModule*> Behaviour_TurnOneEighty::getDependencies()
{
    QList<RobotModule*> ret;
    ret.append( tcl );
    ret.append( xsens );
    return ret;
}

QWidget* Behaviour_TurnOneEighty::createView(QWidget *parent)
{
    Form_TurnOneEighty *form = new Form_TurnOneEighty( this, parent );
    return form;
}

void Behaviour_TurnOneEighty::startBehaviour()
{
    active = true;
    if(!isEnabled()){
        setEnabled(true);
    }

    logger->debug( "Behaviour started" );
    this->setHealthToOk();
    setEnabled( true );
    emit started(this);
    initialHeadingUpdate();

}

void Behaviour_TurnOneEighty::stop()
{
    if(!isActive()){
        logger->info("Not active!");
        return;
    }

    active = false;
    setEnabled( false );
    logger->debug( "Behaviour stopped" );
    emit setAngularSpeed(0.0);

    emit finished( this, true );

}

void Behaviour_TurnOneEighty::terminate()
{
    stop();
    RobotModule::terminate();
}

bool Behaviour_TurnOneEighty::isActive()
{
    return active;
}

void Behaviour_TurnOneEighty::xsensUpdate( RobotModule * )
{
    if(isActive()){
        return;
    }

    if(this->xsens->isEnabled() == true){
        if(this->xsens->getHealthStatus().isHealthOk()){
            this->dataLockerMutex.lock();
            double currentHeading = 0.0;

            currentHeading = this->xsens->getHeading();

            double targetHeading = Angles::deg2deg(initialHeading + this->getSettingsValue("degree").toDouble());
            double diffHeading = Angles::deg2deg(targetHeading - currentHeading);
            addData("heading current", currentHeading);
            addData("heading target", targetHeading);
            addData("heading difference", diffHeading);


            if ( fabs( diffHeading ) < getSettingsValue( "hysteresis", TURN_DEFAULT_HYSTERESIS ).toDouble() )
            {
                emit setAngularSpeed(0.0);
                emit turn180finished();
                QTimer::singleShot(0, this, SLOT(stop()));
            }
            else
            {
                //diffHeading /= this->getSettingsValue("degree").toDouble();
                double angularSpeed = getSettingsValue( "p", TURN_DEFAULT_P ).toDouble() * diffHeading;
                emit setAngularSpeed(angularSpeed);
                qDebug()<<"angulraSpeed" <<angularSpeed;
            }

            dataChanged( this );
            this->dataLockerMutex.unlock();
        }
    } else{
        emit setAngularSpeed(0.0);
        stopOnXsensError();
    }
}

void Behaviour_TurnOneEighty::initialHeadingUpdate()
{
    if(isActive()){
        return;
    }

    this->dataLockerMutex.lock();

    initialHeading = this->xsens->getHeading();

    addData("heading initial", initialHeading);
    dataChanged( this );
    this->dataLockerMutex.unlock();
}

void Behaviour_TurnOneEighty::stopOnXsensError()
{
    if(isActive()){
        return;
    }

    logger->info("Xsens error. Stop!");
    active = false;
    setEnabled( false );
    setHealthToSick("xsens error");
    emit setAngularSpeed(0.0);
    emit finished( this, false );
}

void Behaviour_TurnOneEighty::controlEnabledChanged(bool b){
    if(!b && isActive()){
        logger->info("Disable and deactivate TurnOneEighty");
        stop();
    } else if(!b && !isActive()){
        logger->info("Still deactivated");
    } else if(b && !isActive()){
        logger->info("Enable and activate TurnOneEighty");
        startBehaviour();
    } else {
        logger->info("Still activated");
    }
}
