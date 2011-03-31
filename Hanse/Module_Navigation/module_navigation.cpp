#include "module_navigation.h"

#include <QtGui>
#include "form_navigation.h"
#include <Module_ThrusterControlLoop/module_thrustercontrolloop.h>
#include <Module_SonarLocalization/module_sonarlocalization.h>
//#include <Module_VisualSLAM/module_visualslam.h>
#include <Module_PressureSensor/module_pressuresensor.h>
#include <Module_Compass/module_compass.h>

Module_Navigation::Module_Navigation( QString id,
                                      Module_SonarLocalization *sonarLoc,
                                      Module_ThrusterControlLoop *tcl,
                                      Module_PressureSensor *pressure,
                                      Module_Compass *compass ) :
        RobotModule(id)
{
    this->sonarLoc = sonarLoc;
    this->tcl = tcl;
    this->pressure = pressure;
    this->compass = compass;


}

void Module_Navigation::init()
{
        qRegisterMetaType< QMap<QString,Position> >("QMap<QString,Position>");

    // Connect to signals from the sensors.
    QObject::connect( pressure, SIGNAL( dataChanged(RobotModule*) ),
                      this, SLOT( depthUpdate(RobotModule*) ) );
    QObject::connect( compass, SIGNAL( dataChanged(RobotModule*) ),
                      this, SLOT( headingUpdate(RobotModule*) ) );
    QObject::connect( sonarLoc, SIGNAL( newLocalizationEstimate() ),
                      this, SLOT( sonarPositionUpdate() ) );

    connect(this,SIGNAL(newDepth(float)),tcl,SLOT(setDepth(float)));
    connect(this,SIGNAL(newFFSpeed(float)),tcl,SLOT(setForwardSpeed(float)));
    connect(this,SIGNAL(newANGSpeed(float)),tcl,SLOT(setAngularSpeed(float)));

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
    logger->debug("going to wp");
    currentGoalName = name;
    currentGoalPosition = waypoints[name];
    state = NAV_STATE_GO_TO_GOAL;
    substate = NAV_SUBSTATE_ADJUST_DEPTH;

    Position currentPosition = sonarLoc->getLocalization();
    double dx = currentGoalPosition.getX() - currentPosition.getX();
    double dy = currentGoalPosition.getY() - currentPosition.getY();
    headingToGoal = (atan2( -dx, dy ) * 180 / CV_PI);
    distanceToGoal = sqrt( dx*dx + dy*dy );

    emit newDepth(currentGoalPosition.getZ());
    emit newFFSpeed(.0);
    emit newANGSpeed(.0);
//    tcl->setDepth( currentGoalPosition.getZ() );
//    tcl->setForwardSpeed( .0 );
//    tcl->setAngularSpeed( .0 );

    addData("state", state);
    addData("substate", substate);
    addData("headingToGoal", headingToGoal);
    dataChanged( this );

    emit setNewGoal( currentGoalPosition );
}

void Module_Navigation::clearGoal()
{
    state = NAV_STATE_IDLE;
    substate = NAV_SUBSTATE_ADJUST_DEPTH;

    emit newFFSpeed(.0);
    emit newANGSpeed(.0);
//    tcl->setForwardSpeed( .0 );
//    tcl->setAngularSpeed( .0 );

    addData("state", state);
    addData("substate", substate);
    addData("headingToGoal", .0);
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
                 getSettingsValue( QString( "depth_hysteresis" ), NAV_HYSTERESIS_DEPTH ).toDouble() )
            {
                substate = NAV_SUBSTATE_ADJUST_HEADING;
                addData("substate", substate);
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
    int headingSensor = getSettingsValue( "heading_sensor", 0 ).toInt();
    return;
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
            if ( headingSensor == 0 )
            {
                // Adjust the heading with a P controller.
                if ( fabs( headingToGoal - compassHeading ) >
                     getSettingsValue( "hysteresis_heading", NAV_HYSTERESIS_HEADING ).toDouble() )
                {
                    // positiv: drehhung nach rechts.
                    float val = getSettingsValue( "p_heading", NAV_P_HEADING ).toDouble()
                                * ( headingToGoal - compassHeading );
                    emit newANGSpeed(val);
//                    tcl->setAngularSpeed(  );
                }
                else
                {
                    emit newANGSpeed(.0);
//                    tcl->setAngularSpeed( .0 );
                    float speed = getSettingsValue( "forward_max_speed", NAV_FORWARD_MAX_SPEED ).toFloat();
                    if ( distanceToGoal < getSettingsValue( "forward_max_dist", NAV_FORWARD_MAX_DIST ).toDouble() )
                    {
                        speed -= getSettingsValue( "p_forward", NAV_P_FORWARD ).toDouble() *
                                 ( 1 - ( distanceToGoal / getSettingsValue( "forward_max_dist", NAV_FORWARD_MAX_DIST ).toDouble() ) );
                    }
                    emit newFFSpeed(speed);
//                    tcl->setForwardSpeed( speed );
                    substate = NAV_SUBSTATE_MOVE_FORWARD;
                    initialCompassHeading = compass->getHeading();

                    // Go forward for "forward_time" seconds.
                    QTimer::singleShot( 1000 * getSettingsValue( "forward_time", NAV_FORWARD_TIME ).toDouble(),
                                        this, SLOT( forwardDone() ) );
                }
            }
            break;
        case NAV_SUBSTATE_MOVE_FORWARD:            
            if ( fabs( initialCompassHeading - compassHeading ) >
                 getSettingsValue(QString( "hysteresis_heading" ), NAV_HYSTERESIS_HEADING ).toDouble() )
            {
                float val = getSettingsValue( QString( "p_heading" ), NAV_P_HEADING ).toDouble()
                            * ( initialCompassHeading - compassHeading );
                emit newANGSpeed(val);
//                tcl->setAngularSpeed(  );
            }
            else
            {
                emit newANGSpeed(.0);
//                tcl->setAngularSpeed( .0 );
            }
        }
        break;
    case NAV_STATE_FAILED_TO_GO_TO_GOAL:
    case NAV_STATE_REACHED_GOAL:
        break;
    }

    addData("state", state);
    addData("substate", substate);
    dataChanged( this );
}

void Module_Navigation::vslamPositionUpdate( RobotModule * )
{
    double currentHeading = 0.0; //visSLAM->getLocalization().getYaw();
    int headingSensor = getSettingsValue( "heading_sensor", 0 ).toInt();
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
            if ( headingSensor == 1 )
            {
                // Adjust the heading with a P controller.
                if ( fabs( headingToGoal - currentHeading ) >
                     getSettingsValue( "hysteresis_heading", NAV_HYSTERESIS_HEADING ).toDouble() )
                {
                    // positiv: drehhung nach rechts.
                    float val = getSettingsValue( "p_heading", NAV_P_HEADING ).toDouble()
                                * ( headingToGoal - currentHeading );
//                    tcl->setAngularSpeed(  );
                    emit newANGSpeed(val);
                }
                else
                {
                    emit newANGSpeed(.0);
//                    tcl->setAngularSpeed( .0 );
                    float speed = getSettingsValue( "forward_max_speed", NAV_FORWARD_MAX_SPEED ).toFloat();
                    if ( distanceToGoal < getSettingsValue( "forward_max_dist", NAV_FORWARD_MAX_DIST ).toDouble() )
                    {
                        speed -= getSettingsValue( "p_forward", NAV_P_FORWARD ).toDouble() *
                                 ( 1 - ( distanceToGoal / getSettingsValue( "forward_max_dist", NAV_FORWARD_MAX_DIST ).toDouble() ) );
                    }
                    emit newFFSpeed(speed);
//                    tcl->setForwardSpeed( speed );
                    substate = NAV_SUBSTATE_MOVE_FORWARD;
                    initialCompassHeading = compass->getHeading();

                    // Go forward for "forward_time" seconds.
                    QTimer::singleShot( 1000 * getSettingsValue( "forward_time", NAV_FORWARD_TIME ).toDouble(),
                                        this, SLOT( forwardDone() ) );
                }
            }
            break;
        case NAV_SUBSTATE_MOVE_FORWARD:
            // Check if the goal has already been reached.
            Position currentPosition; // = visSLAM->getLocalization();
            if ( sqrt( ( currentPosition.getX() - currentGoalPosition.getX() ) * ( currentPosition.getX() - currentGoalPosition.getX() ) +
                       ( currentPosition.getY() - currentGoalPosition.getY() ) * ( currentPosition.getY() - currentGoalPosition.getY() ) )
                < getSettingsValue( QString( "hysteresis_goal" ), NAV_HYSTERESIS_GOAL ).toDouble() )
            {
                state = NAV_STATE_REACHED_GOAL;
                emit newFFSpeed(.0);
                emit newANGSpeed(.0);
//                tcl->setForwardSpeed( .0 );
//                tcl->setAngularSpeed( .0 );
                reachedWaypoint( currentGoalName );
            }
            break;
        }
        break;
    case NAV_STATE_FAILED_TO_GO_TO_GOAL:
    case NAV_STATE_REACHED_GOAL:
        break;
    }

    addData("state", state);
    addData("substate", substate);
    dataChanged( this );
}

void Module_Navigation::sonarPositionUpdate()
{
    logger->info("update sonar position");
    double currentHeading = sonarLoc->getLocalization().getYaw();
    int headingSensor = getSettingsValue( "heading_sensor", 0 ).toInt();

    // Adjust the heading with a P controller.
    float diffHeading = headingToGoal - currentHeading;
    while (diffHeading >= 180 ) diffHeading -= 360;
    while (diffHeading < -180 ) diffHeading += 360;

    addData("diffHeading", diffHeading);

    // Check if the goal has already been reached.
    Position currentPosition = sonarLoc->getLocalization();
    if ( sqrt( ( currentPosition.getX() - currentGoalPosition.getX() ) * ( currentPosition.getX() - currentGoalPosition.getX() ) +
               ( currentPosition.getY() - currentGoalPosition.getY() ) * ( currentPosition.getY() - currentGoalPosition.getY() ) )
        < getSettingsValue( QString( "hysteresis_goal" ), NAV_HYSTERESIS_GOAL ).toDouble() )
    {
        state = NAV_STATE_REACHED_GOAL;
        emit newFFSpeed(.0);
        emit newANGSpeed(.0);
//                tcl->setForwardSpeed( .0 );
//                tcl->setAngularSpeed( .0 );
        emit reachedWaypoint( currentGoalName );
    }

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
            if ( headingSensor == 2 )
            {
                if ( fabs( diffHeading ) >
                     getSettingsValue( "hysteresis_heading", NAV_HYSTERESIS_HEADING ).toDouble() )
                {
                    // positiv: drehhung nach rechts.
                    float val = getSettingsValue( "p_heading", NAV_P_HEADING ).toFloat()
                                * diffHeading;
                    if (val > 0.3) val = 0.3;
                    if (val < -0.3) val = -0.3;
                    if (val > 0 && val < 0.2) val = 0.2;
                    if (val < 0 && val > -0.2) val = -0.2;
//                    tcl->setAngularSpeed( );
                    emit newANGSpeed(val);
                }
                else
                {
                    emit newANGSpeed(.0);
//                    tcl->setAngularSpeed( .0 );
                    float speed = getSettingsValue( "forward_max_speed", NAV_FORWARD_MAX_SPEED ).toFloat();
                    if ( distanceToGoal < getSettingsValue( "forward_max_dist", NAV_FORWARD_MAX_DIST ).toDouble() )
                    {
                        speed -= getSettingsValue( "p_forward", NAV_P_FORWARD ).toDouble() *
                                 ( 1 - ( distanceToGoal / getSettingsValue( "forward_max_dist", NAV_FORWARD_MAX_DIST ).toDouble() ) );
                    }
                    emit newFFSpeed(speed);
//                    tcl->setForwardSpeed( speed );
                    substate = NAV_SUBSTATE_MOVE_FORWARD;
                    initialCompassHeading = compass->getHeading();

                    // Go forward for "forward_time" seconds.
                    QTimer::singleShot( 1000 * getSettingsValue("forward_time", NAV_FORWARD_TIME ).toDouble(),
                                        this, SLOT( forwardDone() ) );
                }
            }
            break;
        case NAV_SUBSTATE_MOVE_FORWARD:
            break;
        }
        break;
    case NAV_STATE_FAILED_TO_GO_TO_GOAL:
    case NAV_STATE_REACHED_GOAL:
        break;
    }

    addData("state", state);
    addData("substate", substate);
    dataChanged( this );
}

void Module_Navigation::forwardDone()
{
    emit newFFSpeed(.0);
//    tcl->setForwardSpeed( .0 );
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
    logger->debug("add wp");
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

//    QFile slamFile( slamPath );
//    slamFile.open( QIODevice::WriteOnly );
//    QTextStream slamTS( &slamFile );
//    visSLAM->save( slamTS );
//    slamFile.close();

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

//    QFile slamFile( slamPath );
//    slamFile.open( QIODevice::ReadOnly );
//    QTextStream slamTS( &slamFile );
//    visSLAM->load( slamTS );
//    slamFile.close();

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
