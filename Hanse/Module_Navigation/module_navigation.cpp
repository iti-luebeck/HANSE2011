#include "module_navigation.h"

#include <QtGui>
#include "form_navigation.h"

#include <Module_ThrusterControlLoop/module_thrustercontrolloop.h>
#include <Module_SonarLocalization/module_sonarlocalization.h>
#include <Module_VisualSLAM/module_visualslam.h>
#include <Module_PressureSensor/module_pressuresensor.h>
#include <Module_Compass/module_compass.h>

Module_Navigation::Module_Navigation( QString id,
                                      Module_SonarLocalization *sonarLoc,
                                      Module_VisualSLAM* visSLAM,
                                      Module_ThrusterControlLoop *tcl,
                                      Module_PressureSensor *pressure,
                                      Module_Compass *compass ) :
        RobotModule(id)
{
    this->sonarLoc = sonarLoc;
    this->visSLAM = visSLAM;
    this->tcl = tcl;
    this->pressure = pressure;
    this->compass = compass;

    // Connect to signals from the sensors.
    QObject::connect( pressure, SIGNAL( dataChanged(RobotModule*) ),
                      this, SLOT( depthUpdate(RobotModule*) ) );
    QObject::connect( compass, SIGNAL( dataChanged(RobotModule*) ),
                      this, SLOT( headingUpdate(RobotModule*) ) );
    QObject::connect( visSLAM, SIGNAL( dataChanged(RobotModule*) ),
                      this, SLOT( vslamPositionUpdate(RobotModule*) ) );

    state = NAV_STATE_IDLE;
    substate = NAV_SUBSTATE_ADJUST_DEPTH;
}

void Module_Navigation::reset()
{
    RobotModule::reset();
}

void Module_Navigation::terminate()
{
    RobotModule::terminate();
}

QList<RobotModule*> Module_Navigation::getDependencies()
{
    QList<RobotModule*> ret;
    ret.append( sonarLoc );
    ret.append( visSLAM );
    ret.append( tcl );
    ret.append( pressure );
    ret.append( compass );
    return ret;
}

QWidget* Module_Navigation::createView(QWidget* parent)
{
    Form_Navigation *form = new Form_Navigation( this, parent );
    QObject::connect( this, SIGNAL( updatedWaypoints(QMap<QString,Position>) ),
                      form, SLOT( updateList(QMap<QString,Position>) ) );
    QObject::connect( form, SIGNAL( removedWaypoint(QString) ),
                      SLOT( removeWaypoint(QString) ) );
    updatedWaypoints( waypoints );
    return form;
}

void Module_Navigation::doHealthCheck()
{

}

void Module_Navigation::gotoWayPoint( QString name, Position delta )
{
    currentGoalName = name;
    currentGoalPosition = waypoints[name];
    state = NAV_STATE_GO_TO_GOAL;
    substate = NAV_SUBSTATE_ADJUST_DEPTH;

    Position currentPosition = visSLAM->getLocalization();
    double dx = currentGoalPosition.getX() - currentPosition.getX();
    double dy = currentGoalPosition.getY() - currentPosition.getY();
    headingToGoal = atan2( -dx, dy ) * 180 / CV_PI;

    tcl->setDepth( currentGoalPosition.getZ() );
    tcl->setForwardSpeed( .0 );
    tcl->setAngularSpeed( .0 );

    data["state"] = state;
    data["substate"] = substate;
    data["headingToGoal"] = headingToGoal;
    dataChanged( this );

    setNewGoal( currentGoalPosition );
}

void Module_Navigation::clearGoal()
{
    state = NAV_STATE_IDLE;
    substate = NAV_SUBSTATE_ADJUST_DEPTH;

    tcl->setForwardSpeed( .0 );
    tcl->setAngularSpeed( .0 );

    data["state"] = state;
    data["substate"] = substate;
    data["headingToGoal"] = .0;
    dataChanged( this );
    clearedGoal();
}

void Module_Navigation::depthUpdate( RobotModule * )
{
    float currentDepth = pressure->getDepth();
    switch ( state )
    {
    case NAV_STATE_IDLE:
        break;
    case NAV_STATE_GO_TO_GOAL:
        switch ( substate )
        {
        case NAV_SUBSTATE_ADJUST_DEPTH:
            if ( fabs( currentDepth - currentGoalPosition.getZ() ) <
                 settings.value( QString( "depth_hysteresis" ), NAV_HYSTERESIS_DEPTH ).toDouble() )
            {
                substate = NAV_SUBSTATE_ADJUST_HEADING;
                data["substate"] = substate;
                dataChanged( this );
            }
            break;
        case NAV_SUBSTATE_ADJUST_HEADING:
        case NAV_SUBSTATE_MOVE_FORWARD:
            break;
        }
        break;
    case NAV_STATE_FAILED_TO_GO_TO_GOAL:
    case NAV_STATE_REACHED_GOAL:
        break;
    }
}

void Module_Navigation::headingUpdate( RobotModule * )
{
    double compassHeading = compass->getHeading();
    switch ( state )
    {
    case NAV_STATE_IDLE:
        break;
    case NAV_STATE_GO_TO_GOAL:
        switch ( substate )
        {
        case NAV_SUBSTATE_ADJUST_DEPTH:
            break;
        case NAV_SUBSTATE_ADJUST_HEADING:
            break;
        case NAV_SUBSTATE_MOVE_FORWARD:
            if ( fabs( initialCompassHeading - compassHeading ) >
                 settings.value( QString( "hysteresis_heading" ), NAV_HYSTERESIS_HEADING ).toDouble() )
            {
                tcl->setAngularSpeed( settings.value( QString( "p_heading" ), NAV_P_HEADING ).toDouble()
                                      * ( initialCompassHeading - compassHeading ) );
            }
            else
            {
                tcl->setAngularSpeed( .0 );
            }
        }
        break;
    case NAV_STATE_FAILED_TO_GO_TO_GOAL:
    case NAV_STATE_REACHED_GOAL:
        break;
    }

    data["state"] = state;
    data["substate"] = substate;
    dataChanged( this );
}

void Module_Navigation::vslamPositionUpdate( RobotModule * )
{
    double currentHeading = visSLAM->getLocalization().getYaw();
    switch ( state )
    {
    case NAV_STATE_IDLE:
        break;
    case NAV_STATE_GO_TO_GOAL:
        switch ( substate )
        {
        case NAV_SUBSTATE_ADJUST_DEPTH:
            break;
        case NAV_SUBSTATE_ADJUST_HEADING:
            // Adjust the heading with a P controller.
            if ( fabs( headingToGoal - currentHeading ) >
                 settings.value( QString( "hysteresis_heading"), NAV_HYSTERESIS_HEADING ).toDouble() )
            {
                // positiv: drehhung nach rechts.
                tcl->setAngularSpeed( settings.value( QString( "p_heading" ), NAV_P_HEADING ).toDouble()
                                      * ( headingToGoal - currentHeading ) );
            }
            else
            {
                tcl->setAngularSpeed( .0 );
                tcl->setForwardSpeed( settings.value( QString( "forward_speed" ),
                                                      NAV_FORWARD_SPEED ).toFloat() );
                substate = NAV_SUBSTATE_MOVE_FORWARD;
                initialCompassHeading = compass->getHeading();

                // Go forward for "forward_time" seconds.
                QTimer::singleShot( 1000 * settings.value( QString( "forward_time" ), NAV_FORWARD_TIME ).toDouble(),
                                    this, SLOT( forwardDone() ) );
            }
            break;
        case NAV_SUBSTATE_MOVE_FORWARD:
            // Check if the goal has already been reached.
            Position currentPosition = visSLAM->getLocalization();
            if ( sqrt( ( currentPosition.getX() - currentGoalPosition.getX() ) * ( currentPosition.getX() - currentGoalPosition.getX() ) +
                       ( currentPosition.getY() - currentGoalPosition.getY() ) * ( currentPosition.getY() - currentGoalPosition.getY() ) )
                < settings.value( QString( "hysteresis_goal" ), NAV_HYSTERESIS_GOAL ).toDouble() )
            {
                state = NAV_STATE_REACHED_GOAL;
                tcl->setForwardSpeed( .0 );
                tcl->setAngularSpeed( .0 );
                reachedWaypoint( currentGoalName );
            }
            break;
        }
        break;
    case NAV_STATE_FAILED_TO_GO_TO_GOAL:
    case NAV_STATE_REACHED_GOAL:
        break;
    }

    data["state"] = state;
    data["substate"] = substate;
    dataChanged( this );
}

void Module_Navigation::forwardDone()
{
    tcl->setForwardSpeed( .0 );
    gotoWayPoint( currentGoalName, Position() );
}

Position Module_Navigation::getWayPointPosition( QString name )
{
    return waypoints[name];
}

QMap<QString, Position> Module_Navigation::getWaypoints()
{
    return waypoints;
}

void Module_Navigation::addWaypoint( QString name, Position pos )
{
    waypoints[ name ] = pos;
    updatedWaypoints( waypoints );
}

void Module_Navigation::removeWaypoint( QString name )
{
    waypoints.remove( name );
    updatedWaypoints( waypoints );
}

void Module_Navigation::save( QString path )
{
    QString slamPath = path;
    slamPath.append( "/slam.txt" );

    QFile slamFile( slamPath );
    slamFile.open( QIODevice::WriteOnly );
    QTextStream slamTS( &slamFile );
    visSLAM->save( slamTS );
    slamFile.close();

    QString waypointPath = path;
    waypointPath.append( "/waypoint.txt" );

    QFile waypointFile( waypointPath );
    waypointFile.open( QIODevice::WriteOnly );
    QTextStream waypointTS( &waypointFile );
    this->saveWaypoints( waypointTS );
    waypointFile.close();
}

void Module_Navigation::saveWaypoints( QTextStream &ts )
{
    // Number of waypoints.
    ts << waypoints.size() << endl;

    QList<QString> names = waypoints.keys();
    for ( int i = 0; i < waypoints.size(); i++ )
    {
        QString name = names[i];
        Position pos = waypoints[name];
        ts << name << endl;
        ts << pos.getX() << " " << pos.getY() << " " << pos.getDepth() <<
                " " << pos.getArrivalAngle() << " " << pos.getExitAngle() << endl;
    }
}

void Module_Navigation::load( QString path )
{
    QString slamPath = path;
    slamPath.append( "/slam.txt" );

    QFile slamFile( slamPath );
    slamFile.open( QIODevice::ReadOnly );
    QTextStream slamTS( &slamFile );
    visSLAM->load( slamTS );
    slamFile.close();

    QString waypointPath = path;
    waypointPath.append( "/waypoint.txt" );

    QFile waypointFile( waypointPath );
    waypointFile.open( QIODevice::ReadOnly );
    QTextStream waypointTS( &waypointFile );
    this->loadWaypoints( waypointTS );
    waypointFile.close();
}

void Module_Navigation::loadWaypoints( QTextStream &ts )
{
    int waypointCount = 0;
    ts >> waypointCount;

    waypoints.clear();
    for ( int i = 0; i < waypointCount; i++ )
    {
        QString name;
        double x, y, depth, arrivalAngle, exitAngle;

        ts >> name;
        ts >> x >> y >> depth >> arrivalAngle >> exitAngle;

        Position pos;
        pos.setX( x );
        pos.setY( y );
        pos.setDepth( depth );
        pos.setArrivalAngle( arrivalAngle );
        pos.setExitAngle( exitAngle );

        waypoints[name] = pos;
    }

    updatedWaypoints( waypoints );
}
