#include "behaviour_turnoneeighty.h"
#include <Module_ThrusterControlLoop/module_thrustercontrolloop.h>
#include <Module_Compass/module_compass.h>
#include <Behaviour_TurnOneEighty/form_turnoneeighty.h>
#include <Module_XsensMTi/module_xsensmti.h>

Behaviour_TurnOneEighty::Behaviour_TurnOneEighty( QString id, Module_ThrusterControlLoop* tcl, Module_Compass *compass, Module_XsensMTi *x) :
        RobotBehaviour( id )
{
    this->tcl = tcl;
    this->compass = compass;
    this->xsens = x;

}

void Behaviour_TurnOneEighty::init()
{
    QObject::connect( compass, SIGNAL( dataChanged(RobotModule*) ),
                      this, SLOT( compassUpdate(RobotModule*) ) );
    QObject::connect( xsens, SIGNAL( dataChanged(RobotModule*) ),
                      this, SLOT( compassUpdate(RobotModule*) ) );
    connect(this,SIGNAL(setAngularSpeed(float)),tcl,SLOT(setAngularSpeed(float)));
}

QList<RobotModule*> Behaviour_TurnOneEighty::getDependencies()
{
    QList<RobotModule*> ret;
    ret.append( tcl );
    ret.append( compass );
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
    }
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

void Behaviour_TurnOneEighty::compassUpdate( RobotModule * )
{
    //    qDebug() << "turn compass thread id";
    //    qDebug() << QThread::currentThreadId();
    if ( isActive() )
    {
        this->dataLockerMutex.lock();
        double currentHeading = 0.0;
        if(this->xsens->isEnabled()){
            qDebug("get xsens heading");
            currentHeading = this->xsens->getHeading();
        } else {
            qDebug("get compass heading");
            currentHeading = compass->getHeading();
        }

        double targetHeading = initialHeading + 180;
        if ( targetHeading > 360 )
        {
            targetHeading -= 360;
        }
        double diffHeading = targetHeading - currentHeading;

        if ( diffHeading > 180 )
        {
            diffHeading -= 360;
        }
        if ( diffHeading < -180 )
        {
            diffHeading += 360;
        }

        logger->debug( "current heading %f", currentHeading );
        addData("current_heading", currentHeading);
        logger->debug( "heading difference %f°", diffHeading );
        addData("difference_heading", diffHeading);

        if ( fabs( diffHeading ) < getSettingsValue( "hysteresis", TURN_DEFAULT_HYSTERESIS ).toDouble() )
        {
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
}

void Behaviour_TurnOneEighty::initialHeadingUpdate()
{
    //    qDebug() << "turn thread id";
    //    qDebug() << QThread::currentThreadId();
    this->dataLockerMutex.lock();

    if(this->xsens->isEnabled()){
        qDebug("get xsens initialheading");
        initialHeading = this->xsens->getHeading();
    } else {
        qDebug("get compass initialheading");
        initialHeading = compass->getHeading();
    }
    logger->debug( "initial heading set to %f°", initialHeading );
    addData("initial_heading", initialHeading);
    dataChanged( this );
    this->dataLockerMutex.unlock();
}
