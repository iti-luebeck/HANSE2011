#include "module_navigation.h"

//#include <QtGui>
#include <Module_Navigation/form_navigation.h>
#include <Module_ThrusterControlLoop/module_thrustercontrolloop.h>
#include <Module_SonarLocalization/module_sonarlocalization.h>
#include <Module_PressureSensor/module_pressuresensor.h>
#include <Module_Compass/module_compass.h>
#include <Module_XsensMTi/module_xsensmti.h>

Module_Navigation::Module_Navigation( QString id,
                                      Module_SonarLocalization *sonarLoc,
                                      Module_ThrusterControlLoop *tcl,
                                      Module_PressureSensor *pressure,
                                      Module_Compass *compass,
                                      Module_XsensMTi *mti ) :
        RobotModule(id)
{
    this->sonarLoc = sonarLoc;
    this->tcl = tcl;
    this->pressure = pressure;
    this->compass = compass;
    this->mti = mti;
}

void Module_Navigation::init()
{
    qRegisterMetaType< QMap<QString,Position> >("QMap<QString,Position>");

    // Connect to signals from the sensors.
    QObject::connect( compass, SIGNAL( dataChanged(RobotModule*) ),
                      this, SLOT( compassUpdate(RobotModule*) ) );
    QObject::connect( sonarLoc, SIGNAL( newLocalizationEstimate() ),
                      this, SLOT( sonarPositionUpdate() ) );
    QObject::connect( mti, SIGNAL( dataChanged(RobotModule*) ),
                      this, SLOT( xsensUpdate(RobotModule*) ) );

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
    ret.append(sonarLoc);
    ret.append(tcl);
    ret.append(pressure);
    ret.append(compass);
    ret.append(mti);
    return ret;
}

QWidget* Module_Navigation::createView(QWidget* parent)
{
    Form_Navigation *form = new Form_Navigation( this, parent );
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
    substate = NAV_SUBSTATE_ADJUST_HEADING;

    Position currentPosition = sonarLoc->getLocalization();
    double dx = currentGoalPosition.getX() - currentPosition.getX();
    double dy = currentGoalPosition.getY() - currentPosition.getY();
    headingToGoal = (atan2( -dx, dy ) * 180 / CV_PI);
    distanceToGoal = sqrt( dx*dx + dy*dy );

    emit newDepth(currentGoalPosition.getZ());
    emit newFFSpeed(.0);
    emit newANGSpeed(.0);

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

    addData("state", state);
    addData("substate", substate);
    addData("headingToGoal", .0);
    dataChanged( this );
    clearedGoal();
}

void Module_Navigation::compassUpdate( RobotModule * )
{
    if (!getSettingsValue("enabled").toBool())
        return;

    if (getSettingsValue("use compass", false).toBool()) {
        float compassHeading = compass->getHeading();
        if (state == NAV_STATE_GO_TO_GOAL) {
            if (substate == NAV_SUBSTATE_MOVE_FORWARD) {
                float diffHeading = pi2pi(initialCompassHeading - compassHeading);
                if (fabs(diffHeading) > getSettingsValue(QString("hysteresis_heading"), NAV_HYSTERESIS_HEADING).toFloat()) {
                    float val = getSettingsValue(QString("p_heading"), NAV_P_HEADING).toFloat() * diffHeading;
                    emit newANGSpeed(val);
                } else {
                    emit newANGSpeed(.0);
                }
            }
        }
    }
}

void Module_Navigation::xsensUpdate( RobotModule * )
{
    if (!getSettingsValue("enabled").toBool())
        return;

    if (getSettingsValue("use xsens", true).toBool()) {
        float xsensHeading = mti->getHeading();
        if (state == NAV_STATE_GO_TO_GOAL) {
            if (substate == NAV_SUBSTATE_MOVE_FORWARD) {
                float diffHeading = pi2pi(initialXsensHeading - xsensHeading);
                if (fabs(diffHeading) > getSettingsValue(QString("hysteresis_heading"), NAV_HYSTERESIS_HEADING).toFloat()) {
                    float val = getSettingsValue(QString("p_heading"), NAV_P_HEADING).toFloat() * diffHeading;
                    emit newANGSpeed(val);
                } else {
                    emit newANGSpeed(.0);
                }
            }
        }
    }
}

void Module_Navigation::sonarPositionUpdate()
{
    if (!getSettingsValue("enabled").toBool())
        return;

    // Update all important values.
    float currentDepth = pressure->getDepth();
    Position currentPosition = sonarLoc->getLocalization();
    double currentHeading = currentPosition.getYaw();
    float diffHeading = ang2ang(headingToGoal - currentHeading);

    addData("diffHeading", diffHeading);

    if (state == NAV_STATE_GO_TO_GOAL) {
        // Check if we are close enough to the goal.
        if ( sqrt( ( currentPosition.getX() - currentGoalPosition.getX() ) * ( currentPosition.getX() - currentGoalPosition.getX() ) +
                   ( currentPosition.getY() - currentGoalPosition.getY() ) * ( currentPosition.getY() - currentGoalPosition.getY() ) )
            < getSettingsValue( QString( "hysteresis_goal" ), NAV_HYSTERESIS_GOAL ).toDouble() ) {
            state = NAV_STATE_REACHED_GOAL;
            substate = NAV_SUBSTATE_ADJUST_HEADING;
            emit newFFSpeed(.0);
            emit newANGSpeed(.0);
        } else {
            // First adjust the depth.
            if (substate == NAV_SUBSTATE_ADJUST_DEPTH) {
                if ( fabs( currentDepth - currentGoalPosition.getZ() ) <
                     getSettingsValue( QString( "depth_hysteresis" ), NAV_HYSTERESIS_DEPTH ).toDouble() ) {
                    substate = NAV_SUBSTATE_ADJUST_HEADING;
                }
            }

            // Then adjust the heading towards the goal.
            if (substate == NAV_SUBSTATE_ADJUST_HEADING) {
                // Check whether the heading needs to be corrected.
                if ( fabs(diffHeading) > getSettingsValue("hysteresis_heading", NAV_HYSTERESIS_HEADING).toDouble()) {
                    // positive: rotate right (clockwise)
                    float maxAngSpeed = getSettingsValue("angular_max_speed").toFloat();
                    float minAngSpeed = getSettingsValue("angular_min_speed").toFloat();
                    float val = getSettingsValue( "p_heading", NAV_P_HEADING ).toFloat() * diffHeading;
                    if (val > maxAngSpeed) val = maxAngSpeed;
                    if (val < -maxAngSpeed) val = -maxAngSpeed;
                    if (val > 0 && val < minAngSpeed) val = minAngSpeed;
                    if (val < 0 && val > -minAngSpeed) val = -minAngSpeed;
                    emit newFFSpeed(.0);
                    emit newANGSpeed(val);
                } else {
                    // If heading does not need to be adjusted, move forward.
                    emit newANGSpeed(.0);
                    float speed = getSettingsValue("forward_max_speed", NAV_FORWARD_MAX_SPEED).toFloat();

                    // Move slower if we are close to the goal.
                    if ( distanceToGoal < getSettingsValue("forward_max_dist", NAV_FORWARD_MAX_DIST ).toDouble()) {
                        speed -= getSettingsValue( "p_forward", NAV_P_FORWARD ).toDouble() *
                                 ( 1 - ( distanceToGoal / getSettingsValue( "forward_max_dist", NAV_FORWARD_MAX_DIST ).toDouble() ) );
                    }
                    emit newFFSpeed(speed);

                    substate = NAV_SUBSTATE_MOVE_FORWARD;

                    initialCompassHeading = compass->getHeading();
                    initialXsensHeading = mti->getHeading();

                    // Move forward for "forward_time" seconds.
                    QTimer::singleShot( 1000 * getSettingsValue("forward_time", NAV_FORWARD_TIME ).toDouble(),
                                        this, SLOT( forwardDone() ) );
                }
            }
        }
    }

    // If the goal was reached, the exit orientation must be adjusted.
    if (state == NAV_STATE_REACHED_GOAL) {
        float exitAngle = currentGoalPosition.getExitAngle();
        if (exitAngle >= -180 && exitAngle <= 180) {
            float diffHeadingExit = ang2ang(exitAngle - currentHeading);
            if (fabs(diffHeadingExit) > getSettingsValue("hysteresis_heading", NAV_HYSTERESIS_HEADING).toDouble()) {
                // positive: rotate right (clockwise)
                float maxAngSpeed = getSettingsValue("angular_max_speed").toFloat();
                float minAngSpeed = getSettingsValue("angular_min_speed").toFloat();
                float val = getSettingsValue( "p_heading", NAV_P_HEADING ).toFloat() * diffHeadingExit;
                if (val > maxAngSpeed) val = maxAngSpeed;
                if (val < -maxAngSpeed) val = -maxAngSpeed;
                if (val > 0 && val < minAngSpeed) val = minAngSpeed;
                if (val < 0 && val > -minAngSpeed) val = -minAngSpeed;
                emit newANGSpeed(val);
            } else {
                emit newANGSpeed(.0);
                emit reachedWaypoint( currentGoalName );
            }
        } else {
            emit newANGSpeed(.0);
            emit reachedWaypoint( currentGoalName );
        }
    }

    addData("state", state);
    addData("substate", substate);
    dataChanged( this );
}

void Module_Navigation::forwardDone()
{
    emit newFFSpeed(.0);
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

float Module_Navigation::pi2pi(float ang)
{
    while (ang > M_PI) {
        ang -= 2*M_PI;
    }
    while (ang <= -M_PI) {
        ang += 2*M_PI;
    }
    return ang;
}

float Module_Navigation::ang2ang(float ang)
{
    while (ang > 180) {
        ang -= 360;
    }
    while (ang <= -180) {
        ang += 360;
    }
    return ang;
}
