#include "behaviour_turnoneeighty.h"
#include <Module_ThrusterControlLoop/module_thrustercontrolloop.h>
#include <Module_Compass/module_compass.h>
#include <Behaviour_TurnOneEighty/form_turnoneeighty.h>

Behaviour_TurnOneEighty::Behaviour_TurnOneEighty( QString id, Module_ThrusterControlLoop* tcl, Module_Compass *compass ) :
        RobotBehaviour( id )
{
    this->tcl = tcl;
    this->compass = compass;
    QObject::connect( compass, SIGNAL( dataChanged(RobotModule*) ),
                      this, SLOT( compassUpdate(RobotModule*) ) );
}

QList<RobotModule*> Behaviour_TurnOneEighty::getDependencies()
{
    QList<RobotModule*> ret;
    ret.append( tcl );
    ret.append( compass );
    return ret;
}

QWidget* Behaviour_TurnOneEighty::createView(QWidget *parent)
{
    Form_TurnOneEighty *form = new Form_TurnOneEighty( this, parent );
    return form;
}

void Behaviour_TurnOneEighty::start()
{
    if( !isEnabled() )
    {
        logger->debug( "Behaviour started" );
        this->setHealthToOk();
        setEnabled( true );
    }
}

void Behaviour_TurnOneEighty::stop()
{
    if ( isActive() )
    {
        logger->debug( "Behaviour stopped" );
       tcl->setAngularSpeed(0.0);
       setEnabled( false );
       emit finished( this, true );
   }
}

bool Behaviour_TurnOneEighty::isActive()
{
    return isEnabled();
}

void Behaviour_TurnOneEighty::compassUpdate( RobotModule * )
{
    if ( isActive() )
    {
        double currentHeading = compass->getHeading();
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
        data["current_heading"] = currentHeading;
        logger->debug( "heading difference %f°", diffHeading );
        data["difference_heading"] = diffHeading;

        if ( fabs( diffHeading ) < settings.value( "hysteresis", TURN_DEFAULT_HYSTERESIS ).toDouble() )
        {
            stop();
        }
        else
        {
            diffHeading /= 180;
            double angularSpeed = settings.value( "p", TURN_DEFAULT_P ).toDouble() * diffHeading;
            tcl->setAngularSpeed( angularSpeed );
        }

        dataChanged( this );
    }
}

void Behaviour_TurnOneEighty::initialHeadingUpdate()
{
    initialHeading = compass->getHeading();
    logger->debug( "initial heading set to %f°", initialHeading );
    data["initial_heading"] = initialHeading;
    dataChanged( this );
}
