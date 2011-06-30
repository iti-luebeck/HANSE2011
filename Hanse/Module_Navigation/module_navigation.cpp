#include "module_navigation.h"

//#include <QtGui>
#include <Module_Navigation/form_navigation.h>
#include <Module_ThrusterControlLoop/module_thrustercontrolloop.h>
#include <Module_SonarLocalization/module_sonarlocalization.h>
#include <Module_PressureSensor/module_pressuresensor.h>
#include <Module_XsensMTi/module_xsensmti.h>
#include <Framework/Angles.h>

Module_Navigation::Module_Navigation( QString id,
                                      Module_SonarLocalization *sonarLoc,
                                      Module_ThrusterControlLoop *tcl,
                                      Module_PressureSensor *pressure,
                                      Module_XsensMTi *mti ) :
RobotModule(id)
{
    this->sonarLoc = sonarLoc;
    this->tcl = tcl;
    this->pressure = pressure;
    this->mti = mti;

    headingToGoal = 0;
    distanceToGoal = 0;

    // For driving straight.
    initialXsensHeading = 0;

    // For not turning too much.
    adjustHeadingInitialXsens = 0;
    diffHeading = 0;

    trustingCurrentPosition = false;
}

void Module_Navigation::init()
{
    // Connect to signals from the sensors.
    QObject::connect( sonarLoc, SIGNAL( newLocalizationEstimate() ),
                      this, SLOT( sonarPositionUpdate() ) );
    QObject::connect( mti, SIGNAL( dataChanged(RobotModule*) ),
                      this, SLOT( xsensUpdate(RobotModule*) ) );

    connect(this,SIGNAL(newDepth(float)),tcl,SLOT(setDepth(float)));
    connect(this,SIGNAL(newFFSpeed(float)),tcl,SLOT(setForwardSpeed(float)));
    connect(this,SIGNAL(newANGSpeed(float)),tcl,SLOT(setAngularSpeed(float)));

    state = NAV_STATE_IDLE;
    substate = NAV_SUBSTATE_ADJUST_DEPTH;
    hasActiveGoal = false;
    trustingCurrentPosition = false;
}

void Module_Navigation::reset()
{
    RobotModule::reset();
    trustingCurrentPosition = false;
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

void Module_Navigation::gotoWayPoint(QString name)
{
    path.clear();
    path.push_back(name);
    navigateToNextWaypoint();
}

void Module_Navigation::gotoPath(QList<QString> waypoints)
{
    path = waypoints;
    navigateToNextWaypoint();
}

void Module_Navigation::navigateToNextWaypoint()
{
    if (path.isEmpty()) {
        hasActiveGoal = false;
        state = NAV_STATE_IDLE;
        substate = NAV_SUBSTATE_DONE;

        addData("state", state);
        addData("substate", substate);
        addData("heading absolute position to goal", .0);
        addData("current goal", "");
        emit dataChanged(this);
    } else {
        hasActiveGoal = true;

        currentGoalName = path.front();
        path.pop_front();
        addData("current goal", currentGoalName);

        currentGoal = waypoints[currentGoalName];
        emit setNewGoal(currentGoal);

        navigateToCurrentWaypoint();
    }
}

void Module_Navigation::navigateToCurrentWaypoint()
{    
    if (!getSettingsValue("enabled").toBool()) {
        return;
    }

    state = NAV_STATE_GO_TO_GOAL;
    substate = NAV_SUBSTATE_ADJUST_HEADING;

    Position currentPosition = sonarLoc->getLocalization();
    double dx = currentGoal.posX - currentPosition.getX();
    double dy = currentGoal.posY - currentPosition.getY();
    headingToGoal = atan2(dx, dy) * 180 / CV_PI + 90;
    distanceToGoal = sqrt(dx*dx + dy*dy);

    emit newDepth(currentGoal.depth);
    emit newFFSpeed(.0);
    emit newANGSpeed(.0);

    addData("state", state);
    addData("substate", substate);
    addData("heading absolute position to goal", headingToGoal);
    dataChanged(this);
}

void Module_Navigation::pause()
{
    // Set pause state.
    pauseState = state;
    pauseSubstate = substate;
    state = NAV_STATE_PAUSED;
    addData("state", state);

    // Reset thruster values.
    emit newFFSpeed(.0);
    emit newANGSpeed(.0);

    dataChanged(this);
}

void Module_Navigation::resume()
{
    state = pauseState;
    substate = pauseSubstate;

    if (hasActiveGoal) {
        navigateToCurrentWaypoint();
    }
    addData("state", state);
    addData("substate", substate);
    dataChanged(this);
}

void Module_Navigation::clearGoal()
{
    navigateToNextWaypoint();
    emit clearedGoal();
    emit newFFSpeed(.0);
    emit newANGSpeed(.0);
}

void Module_Navigation::clearPath()
{
    state = NAV_STATE_IDLE;
    substate = NAV_SUBSTATE_DONE;
    path.clear();

    emit newFFSpeed(.0);
    emit newANGSpeed(.0);

    hasActiveGoal = false;

    addData("state", state);
    addData("substate", substate);
    addData("heading absolute position to goal", .0);
    addData("current goal", "");
    dataChanged( this );
    emit clearedGoal();
}

void Module_Navigation::xsensUpdate( RobotModule * )
{
    if (!getSettingsValue("enabled").toBool()) {
        return;
    }

    if (!trustingCurrentPosition) {
        return;
    }

    //--------------------------------------------------------------------
    //  XSENS IN NAVIGATION
    //
    // We use the xsens sensor for navigation in two ways
    //  1. when adjusting the heading towards the goal, we stop if the
    //     xsens indicates that we have turned enough
    //  2. when driving forward, we use the xsens to move as straight as
    //     possible
    //--------------------------------------------------------------------
    if (getSettingsValue("use xsens", true).toBool()) {
        float xsensHeading = mti->getHeading();
        if (state == NAV_STATE_GO_TO_GOAL || state == NAV_STATE_REACHED_GOAL) {

            if (substate == NAV_SUBSTATE_ADJUST_HEADING) {
                double diffXsens = Angles::deg2deg(adjustHeadingInitialXsens - xsensHeading);
                addData("heading xsens to position", diffXsens);

                if (fabs(Angles::deg2deg(diffHeading - diffXsens)) < 5) {
                    emit newANGSpeed(0.0f);
                    addData("speed angular", 0.0f);
                } else {
                    float maxAngSpeed = getSettingsValue("angular_max_speed").toFloat();
                    float minAngSpeed = getSettingsValue("angular_min_speed").toFloat();
                    float val = getSettingsValue( "p_heading", NAV_P_HEADING ).toFloat() * (diffHeading - diffXsens);
                    if (val > maxAngSpeed) val = maxAngSpeed;
                    if (val < -maxAngSpeed) val = -maxAngSpeed;
                    if (val > 0 && val < minAngSpeed) val = minAngSpeed;
                    if (val < 0 && val > -minAngSpeed) val = -minAngSpeed;

                    emit newANGSpeed(val);
                    addData("speed angular", val);
                }
            }

            if (substate == NAV_SUBSTATE_MOVE_FORWARD) {
                float headingError = Angles::deg2deg(xsensHeading - initialXsensHeading);
                float val = getSettingsValue(QString("p_heading"), NAV_P_HEADING).toFloat() * headingError;
                emit newANGSpeed(val);
                addData("speed angular", val);
            }
        }
    }
}

Position Module_Navigation::getCurrentPosition()
{
    return sonarLoc->getLocalization();
}

bool Module_Navigation::hasGoal()
{
    return hasActiveGoal;
}

Waypoint Module_Navigation::getCurrentGoal()
{
    return currentGoal;
}

void Module_Navigation::sonarPositionUpdate()
{
    if (!getSettingsValue("enabled").toBool()) {
        return;
    }

    if (substate == NAV_SUBSTATE_MOVE_FORWARD) {
        substate = NAV_SUBSTATE_ADJUST_HEADING;
    }

    // Update all important values.
    float currentDepth = pressure->getDepth();
    currentPosition = sonarLoc->getLocalization();
    double currentHeading = currentPosition.getYaw();

    // Adjust heading to compensate for movement after the last observation was captured.
    if (getSettingsValue("use xsens", true).toBool()) {
        currentHeading -= (mti->getHeading() - sonarLoc->sonarEchoFilter().getLastObservationHeading());
        currentHeading = Angles::deg2deg(currentHeading);
    }

    double dx = currentGoal.posX - currentPosition.getX();
    double dy = currentGoal.posY - currentPosition.getY();
    headingToGoal = -atan2(dx, dy) * 180 / CV_PI;
    distanceToGoal = sqrt(dx*dx + dy*dy);

    addData("heading position to goal", headingToGoal);

    float forwardSpeed = .0;
    float angularSpeed = .0;

    // Check whether there were enough matches to trust the localization estimate.
    if (sonarLoc->getLocalizationConfidence() < 0.7f) {
        // Set speed to 0 to help the localization.
        trustingCurrentPosition = false;
    } else {
        trustingCurrentPosition = true;

        //--------------------------------------------------------------------
        //  STATE_IDLE
        //
        // Do nothing if we are idle.
        //--------------------------------------------------------------------
        if (state == NAV_STATE_IDLE) { }


        //--------------------------------------------------------------------
        //  STATE_PAUSED
        //
        // Do nothing if we are paused.
        //--------------------------------------------------------------------
        if (state == NAV_STATE_PAUSED) { }


        //--------------------------------------------------------------------
        //  STATE_GO_TO_GOAL
        //
        // Navigates to a goal:
        //  1. adjust depth
        //  2. adjust heading to goal
        //  3. move forward for some time
        //  4. start over
        //--------------------------------------------------------------------
        if (state == NAV_STATE_GO_TO_GOAL) {
            diffHeading = Angles::deg2deg(headingToGoal - currentHeading);
            addData("heading relative position to goal", diffHeading);

            // Check if we are close enough to the goal.
            if ( sqrt( ( currentPosition.getX() - currentGoal.posX ) * ( currentPosition.getX() - currentGoal.posX ) +
                       ( currentPosition.getY() - currentGoal.posY ) * ( currentPosition.getY() - currentGoal.posY ) )
                < getSettingsValue( QString( "hysteresis_goal" ), NAV_HYSTERESIS_GOAL ).toDouble() ) {
                state = NAV_STATE_REACHED_GOAL;
                substate = NAV_SUBSTATE_ADJUST_HEADING;
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
                    adjustHeadingInitialXsens = mti->getHeading();

                    // Check whether the heading needs to be corrected.
                    if ( fabs(diffHeading) > getSettingsValue("hysteresis_heading", NAV_HYSTERESIS_HEADING).toDouble()) {

                        // positive: rotate right (clockwise)
                        float maxAngSpeed = getSettingsValue("angular_max_speed").toFloat();
                        float minAngSpeed = getSettingsValue("angular_min_speed").toFloat();
                        float val = - getSettingsValue( "p_heading", NAV_P_HEADING ).toFloat() * diffHeading;
                        if (val > maxAngSpeed) val = maxAngSpeed;
                        if (val < -maxAngSpeed) val = -maxAngSpeed;
                        if (val > 0 && val < minAngSpeed) val = minAngSpeed;
                        if (val < 0 && val > -minAngSpeed) val = -minAngSpeed;

                        angularSpeed = val;
                        forwardSpeed = 0.0f;

                    } else {
                        substate = NAV_SUBSTATE_MOVE_FORWARD;
                    }
                }

                if (substate == NAV_SUBSTATE_MOVE_FORWARD) {

                    // If heading does not need to be adjusted, move forward.
                    float speed = getSettingsValue("forward_max_speed", NAV_FORWARD_MAX_SPEED).toFloat();

                    // Move slower if we are close to the goal.
                    if ( distanceToGoal < getSettingsValue("forward_max_dist", NAV_FORWARD_MAX_DIST ).toDouble()) {
                        speed -= 0.5 * getSettingsValue("forward_max_speed", NAV_FORWARD_MAX_SPEED).toFloat() *
                                 (1 - distanceToGoal / getSettingsValue( "forward_max_dist", NAV_FORWARD_MAX_DIST ).toDouble());
                    }

                    angularSpeed = 0.0f;
                    forwardSpeed = speed;

                    initialXsensHeading = mti->getHeading() - diffHeading;

                    // Move forward for "forward_time" seconds.
//                    QTimer::singleShot( 1000 * getSettingsValue("forward_time", NAV_FORWARD_TIME ).toDouble(),
//                                        this, SLOT( forwardDone() ) );
                }
            }
        }

        //--------------------------------------------------------------------
        //  STATE_REACHED_GOAL
        //
        // We reached the current goal position:
        //  1. if needed, adjust the robot heading to match the desired exit
        //     angle
        //  2. stop all motors and navigate to next waypoint
        //--------------------------------------------------------------------
        if (state == NAV_STATE_REACHED_GOAL) {

            if (currentGoal.useExitAngle) {
                adjustHeadingInitialXsens = mti->getHeading();
                float exitAngle = currentGoal.exitAngle;
                diffHeading = Angles::deg2deg(exitAngle - currentHeading);
                if (fabs(diffHeading) > getSettingsValue("hysteresis_heading", NAV_HYSTERESIS_HEADING).toDouble()) {
                    // positive: rotate right (clockwise)
                    float maxAngSpeed = getSettingsValue("angular_max_speed").toFloat();
                    float minAngSpeed = getSettingsValue("angular_min_speed").toFloat();
                    float val = getSettingsValue( "p_heading", NAV_P_HEADING ).toFloat() * diffHeading;
                    if (val > maxAngSpeed) val = maxAngSpeed;
                    if (val < -maxAngSpeed) val = -maxAngSpeed;
                    if (val > 0 && val < minAngSpeed) val = minAngSpeed;
                    if (val < 0 && val > -minAngSpeed) val = -minAngSpeed;

                    angularSpeed = val;
                    forwardSpeed = 0.0f;
                } else {
                    angularSpeed = 0.0f;
                    forwardSpeed = 0.0f;
                    emit reachedWaypoint( currentGoalName );

                    navigateToNextWaypoint();
                }
            } else {
                angularSpeed = 0.0f;
                forwardSpeed = 0.0f;
                emit reachedWaypoint( currentGoalName );

                navigateToNextWaypoint();
            }
        }
    }

    emit newANGSpeed(angularSpeed);
    emit newFFSpeed(forwardSpeed);

    addData("state", state);
    addData("substate", substate);
    addData("speed angular", angularSpeed);
    addData("speed forward", forwardSpeed);
    dataChanged( this );
}

void Module_Navigation::forwardDone()
{
    emit newFFSpeed(.0);
    substate = NAV_SUBSTATE_ADJUST_HEADING;
    addData("substate", substate);
    dataChanged( this );
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

void Module_Navigation::saveToSettings()
{
    // Clear old waypoints.
    QSettings newSettings(QSettings::IniFormat, QSettings::UserScope, "HanseCfg", getId());
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

void Module_Navigation::loadFromSettings()
{
    // Clear waypoint list.
    waypoints.clear();

    // Get all waypoints.
    QSettings newSettings(QSettings::IniFormat, QSettings::UserScope, "HanseCfg", getId());
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
    if(this->waypoints.contains(name)== true){
        Waypoint goal = waypoints[name];
        double dx = goal.posX - currentPosition.getX();
        double dy = goal.posY - currentPosition.getY();
        double currentDistanceToGoal = sqrt( dx*dx + dy*dy );
        return currentDistanceToGoal;
    } else {
        return -1.0;
    }
}

double Module_Navigation::getAlpha(QString name1, QString name2) {
     double alpha = 0.0;
     if (this->isEnabled()) {
            if (waypoints.contains(name1) &&  waypoints.contains(name2)) {

            Position currentPosition = sonarLoc->getLocalization();
            double px = currentPosition.getX();
            double py = currentPosition.getY();


            Waypoint goal1 = waypoints[name1];
            double g1x = goal1.posX;
            double g1y = goal1.posY;


            Waypoint goal2 = waypoints[name2];
            double g2x = goal2.posX;
            double g2y = goal2.posY;

            double v1x = g1x - g2x;
            double v1y = g1y - g2y;

            double v2x = px - g2x;
            double v2y = py - g2y;

            double theta1 = atan2(v1x, v1y);
            double theta2 = atan2(v2x, v2y);

            alpha = Angles::pi2deg(theta2 - theta1);

            addData("Alpha:", alpha);
            emit dataChanged(this);
        }
    }
    return alpha;
}
