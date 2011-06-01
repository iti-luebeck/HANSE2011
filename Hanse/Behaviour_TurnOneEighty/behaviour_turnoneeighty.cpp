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
    QObject::connect( xsens, SIGNAL( dataChanged(RobotModule*) ),
                      this, SLOT( xsensUpdate(RobotModule*) ) );
    connect(this,SIGNAL(setAngularSpeed(float)),tcl,SLOT(setAngularSpeed(float)));
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
    if( !isEnabled() )
    {
        logger->debug( "Behaviour started" );
        this->setHealthToOk();
        setEnabled( true );
        emit started(this);
        initialHeadingUpdate();
    }
}

void Behaviour_TurnOneEighty::stop()
{
    if ( isActive() )
    {
        logger->debug( "Behaviour stopped" );
        emit setAngularSpeed(0.0);
        //       tcl->setAngularSpeed(0.0);
        setEnabled( false );
        emit finished( this, true );
    } else {
        logger->debug("Behaviour not enabled, cant stop");
    }

void Behaviour_TurnOneEighty::terminate()
{
    //    QTimer::singleShot(0,this,SLOT(stop()));
    this->stop();
    RobotModule::terminate();
}

bool Behaviour_TurnOneEighty::isActive()
{
    return isEnabled();
}

void Behaviour_TurnOneEighty::xsensUpdate( RobotModule * )
{
    if (this->isEnabled() == false){
        return;
    }
    //    qDebug() << QThread::currentThreadId();
    if(this->xsens->isEnabled()){
        if(this->xsens->getHealthStatus().isHealthOk()){
            this->dataLockerMutex.lock();
            double currentHeading = 0.0;

            currentHeading = this->xsens->getHeading();

            double targetHeading = initialHeading + 180;
            double diffHeading = Angles::deg2deg(targetHeading - currentHeading);

            logger->debug( "current heading %f", currentHeading );
            addData("current_heading", currentHeading);
            logger->debug( "heading difference %f°", diffHeading );
            addData("difference_heading", diffHeading);

            if ( fabs( diffHeading ) < getSettingsValue( "hysteresis", TURN_DEFAULT_HYSTERESIS ).toDouble() )
            {
                emit setAngularSpeed(0.0);
                stop();
            }
            else
            {
                diffHeading /= 180;
                double angularSpeed = getSettingsValue( "p", TURN_DEFAULT_P ).toDouble() * diffHeading;
                emit setAngularSpeed(angularSpeed);
                //            tcl->setAngularSpeed( angularSpeed );
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
    if (this->isEnabled() == false){
        logger->info("Not enabled!");
        return;
    }
    this->dataLockerMutex.lock();

    initialHeading = this->xsens->getHeading();

    logger->debug( "initial heading set to %f°", initialHeading );
    addData("initial_heading", initialHeading);
    dataChanged( this );
    this->dataLockerMutex.unlock();
}

void Behaviour_TurnOneEighty::stopOnXsensError()
{
    logger->info("Xsens error. Stop!");
    this->setEnabled(false);
    setHealthToSick("xsens error");
    emit setAngularSpeed(0.0);
    emit finished( this, false );
}

void Behaviour_TurnOneEighty::controlEnabledChanged(bool b){
    if(b == false){
        logger->info("No longer enabled!");
        QTimer::singleShot(0, this, SLOT(stop()));
    }
}
