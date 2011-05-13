#include "module_navigation.h"

//#include <QtGui>
#include <Module_Navigation/form_navigation.h>
#include <Module_ThrusterControlLoop/module_thrustercontrolloop.h>
#include <Module_SonarLocalization/module_sonarlocalization.h>
#include <Module_PressureSensor/module_pressuresensor.h>
#include <Module_Compass/module_compass.h>
#include <Module_XsensMTi/module_xsensmti.h>
#include <Framework/Angles.h>

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

void Module_Navigation::gotoWayPoint( QString name )
{
    currentGoalName = name;
    currentGoal = waypoints[name];
    state = NAV_STATE_GO_TO_GOAL;
    substate = NAV_SUBSTATE_ADJUST_HEADING;

    Position currentPosition = sonarLoc->getLocalization();
    double dx = currentGoal.posX - currentPosition.getX();
    double dy = currentGoal.posY - currentPosition.getY();
    headingToGoal = (atan2( -dx, dy ) * 180 / CV_PI);
    distanceToGoal = sqrt( dx*dx + dy*dy );

    emit newDepth(currentGoal.depth);
    emit newFFSpeed(.0);
    emit newANGSpeed(.0);

    addData("state", state);
    addData("substate", substate);
    addData("headingToGoal", headingToGoal);
    dataChanged( this );

    emit setNewGoal( currentGoal );
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
                float diffHeading = Angles::pi2pi(initialCompassHeading - compassHeading);
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
                float diffHeading = Angles::pi2pi(initialXsensHeading - xsensHeading);
                if (fabs(diffHeading) > getSettingsValue(QString("hysteresis_heading"), NAV_HYSTERESIS_HEADING).toFloat()) {
                    float val = getSettingsValue(QString("p_heading"), NAV_P_HEADING).toFloat() * diffHeading;
                    emit newANGSpeed(val);
                } else {
                    emit newANGSpeed(.0);
                }
            }

            if (substate == NAV_SUBSTATE_ADJUST_HEADING) {
                double currentXsensHeading = mti->getHeading();
                double diffXsens = Angles::deg2deg(currentXsensHeading - adjustHeadingInitialXsens);
                addData("xsens heading to last localization", diffXsens);

                if (fabs(diffXsens - diffHeading) < 10) {
                    emit newANGSpeed(.0);
                }
            }
        }
    }
}

Position Module_Navigation::getCurrentPosition()
{
    return currentPosition;
}

void Module_Navigation::sonarPositionUpdate()
{
    if (!getSettingsValue("enabled").toBool())
        return;

    // Update all important values.
    float currentDepth = pressure->getDepth();
    currentPosition = sonarLoc->getLocalization();
    double currentHeading = currentPosition.getYaw();
    diffHeading = Angles::deg2deg(headingToGoal - currentHeading);

    addData("heading to goal", diffHeading);

    if (state == NAV_STATE_GO_TO_GOAL) {
        // Check if we are close enough to the goal.
        if ( sqrt( ( currentPosition.getX() - currentGoal.posX ) * ( currentPosition.getX() - currentGoal.posX ) +
                   ( currentPosition.getY() - currentGoal.posY ) * ( currentPosition.getY() - currentGoal.posY ) )
            < getSettingsValue( QString( "hysteresis_goal" ), NAV_HYSTERESIS_GOAL ).toDouble() ) {
            state = NAV_STATE_REACHED_GOAL;
            substate = NAV_SUBSTATE_ADJUST_HEADING;
            emit newFFSpeed(.0);
            emit newANGSpeed(.0);
        } else {
            // First adjust the depth.
            if (substate == NAV_SUBSTATE_ADJUST_DEPTH) {
                if ( fabs( currentDepth - currentGoal.depth ) <
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
                    adjustHeadingInitialXsens = mti->getHeading();
                    addData("set speed", val);
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

    // If the goal was reached, the exit orientation may need to be adjusted.
    if (state == NAV_STATE_REACHED_GOAL) {
        if (currentGoal.useExitAngle) {
            float exitAngle = currentGoal.exitAngle;
            float diffHeadingExit = Angles::deg2deg(exitAngle - currentHeading);
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
    gotoWayPoint(currentGoalName);
}

Waypoint Module_Navigation::getWayPointPosition( QString name )
{
    return waypoints[name];
}

QMap<QString, Waypoint> Module_Navigation::getWaypoints()
{
    return waypoints;
}

void Module_Navigation::addWaypoint( QString name, Waypoint pos )
{
    waypoints[name] = pos;
    updatedWaypoints( waypoints );
}

void Module_Navigation::removeWaypoint( QString name )
{
    waypoints.remove( name );
    updatedWaypoints( waypoints );
}

void Module_Navigation::save( QString path )
{
//    QString waypointPath = path;
//    waypointPath.append( "/waypoint.txt" );

//    QFile waypointFile( waypointPath );
//    waypointFile.open( QIODevice::WriteOnly );
//    QTextStream waypointTS( &waypointFile );
//    this->saveWaypoints( waypointTS );
//    waypointFile.close();
}

//void Module_Navigation::saveWaypoints( QTextStream &ts )
//{
//    // Number of waypoints.
//    ts << waypoints.size() << endl;

//    QList<QString> names = waypoints.keys();
//    for ( int i = 0; i < waypoints.size(); i++ )
//    {
//        QString name = names[i];
//        Position pos = waypoints[name];
//        ts << name << endl;
//        ts << pos.getX() << " " << pos.getY() << " " << pos.getDepth() <<
//                " " << pos.getArrivalAngle() << " " << pos.getExitAngle() << endl;
//    }
//}

void Module_Navigation::saveToSettings()
{
    // Clear old waypoints.
    QSettings newSettings(QSettings::IniFormat, QSettings::UserScope, "ITI", "Hanse");
    newSettings.beginGroup(getId());
    newSettings.beginGroup("waypoints");
    newSettings.remove("");

    // Add new waypoints.
    QList<QString> names = waypoints.keys();
    for ( int i = 0; i < waypoints.size(); i++ ) {
        QString name = names[i];
        Waypoint waypoint = waypoints[name];
        QVariant waypointVariant;
        waypointVariant.setValue<Waypoint>(waypoint);
        newSettings.setValue(name, waypointVariant);
    }
    newSettings.endGroup();
}

void Module_Navigation::load( QString path )
{
//    QString waypointPath = path;
//    waypointPath.append( "/waypoint.txt" );

//    QFile waypointFile( waypointPath );
//    waypointFile.open( QIODevice::ReadOnly );
//    QTextStream waypointTS( &waypointFile );
//    this->loadWaypoints( waypointTS );
//    waypointFile.close();
}

//void Module_Navigation::loadWaypoints( QTextStream &ts )
//{
//    int waypointCount = 0;
//    ts >> waypointCount;

//    waypoints.clear();
//    for ( int i = 0; i < waypointCount; i++ ) {
//        QString name;
//        double x, y, depth, arrivalAngle, exitAngle;

//        ts >> name;
//        ts >> x >> y >> depth >> arrivalAngle >> exitAngle;

//        Position pos;
//        pos.setX( x );
//        pos.setY( y );
//        pos.setDepth( depth );
//        pos.setArrivalAngle( arrivalAngle );
//        pos.setExitAngle( exitAngle );

//        waypoints[name] = pos;
//    }

//    updatedWaypoints( waypoints );
//}

void Module_Navigation::loadFromSettings()
{
    // Clear waypoint list.
    waypoints.clear();

    // Get all waypoints.
    QSettings newSettings(QSettings::IniFormat, QSettings::UserScope, "ITI", "Hanse");
    newSettings.beginGroup(getId());
    newSettings.beginGroup("waypoints");
    QStringList keys = newSettings.childKeys();

    // Add waypoints.
    for ( int i = 0; i < keys.size(); i++ ) {
        QVariant waypointVariant = newSettings.value(keys[i]);
        if (waypointVariant.canConvert<Waypoint>()) {
            waypoints[keys[i]] = waypointVariant.value<Waypoint>();
        }
    }
    newSettings.endGroup();
    updatedWaypoints( waypoints );
}

double Module_Navigation::getDistance(QString name){
    Position currentPosition = sonarLoc->getLocalization();
    Waypoint goal = waypoints[name];
    double dx = goal.posX - currentPosition.getX();
    double dy = goal.posY - currentPosition.getY();
    double currentDistanceToGoal = sqrt( dx*dx + dy*dy );
    return currentDistanceToGoal;
}
