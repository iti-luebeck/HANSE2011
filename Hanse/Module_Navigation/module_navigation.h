#ifndef MODULE_NAVIGATION_H
#define MODULE_NAVIGATION_H

#include <QtCore>
#include <QGraphicsScene>
#include <Framework/robotmodule.h>
#include <Framework/position.h>
#include "waypoint.h"

#define NAV_STATE_IDLE                  "idle"
#define NAV_STATE_PAUSED                "paused"
#define NAV_STATE_GO_TO_GOAL            "go to goal"
#define NAV_STATE_FAILED_TO_GO_TO_GOAL  "go to goal failed"
#define NAV_STATE_REACHED_GOAL          "reached goal"

#define NAV_SUBSTATE_ADJUST_DEPTH       "adjust depth"
#define NAV_SUBSTATE_ADJUST_HEADING     "adjust heading"
#define NAV_SUBSTATE_MOVE_FORWARD       "move forward"
#define NAV_SUBSTATE_DONE               "done"

#define NAV_P_HEADING           0.005
#define NAV_HYSTERESIS_HEADING  10
#define NAV_HYSTERESIS_GOAL     0.5
#define NAV_HYSTERESIS_DEPTH    0.1
#define NAV_P_FORWARD           0.02
#define NAV_FORWARD_MAX_SPEED   0.2
#define NAV_FORWARD_MAX_DIST    4
#define NAV_FORWARD_TIME        5

class Module_SonarLocalization;
class Module_ThrusterControlLoop;
class Module_PressureSensor;
class Module_XsensMTi;

class MapWidget;

class Module_Navigation : public RobotModule
{
    friend class MapWidget;
    Q_OBJECT
public:
    Module_Navigation( QString id,
                       Module_SonarLocalization *sonarLoc,
                       Module_ThrusterControlLoop* tcl,
                       Module_PressureSensor *pressure,
                       Module_XsensMTi *mti );

    QWidget* createView(QWidget* parent);

    QList<RobotModule*> getDependencies();

    /**
      * get the list of all waypoints.
      */
    QMap<QString, Waypoint> getWaypoints();

    /**
      * get the position of a waypoint
      */
    Waypoint getWayPointPosition(QString name);

    /**
      * Returns the euclidian distance from current position to (goal)position of the name
      */
    double getDistance(QString name);

    Position getCurrentPosition();

    bool hasGoal();

    Waypoint getCurrentGoal();

private:
    void init();

    /**
      * Starts navigation state machine for the current waypoint.
      */
    void navigateToCurrentWaypoint();

    /**
      * Starts navigation for the next waypoint from the list of waypoints.
      */
    void navigateToNextWaypoint();

public slots:
    /**
      * move the robot the given waypoint.
      */
    void gotoWayPoint(QString name);

    /**
      * Moves the robot along a given path (i.e. list of waypoints).
      */
    void gotoPath(QList<QString> waypoints);

    /**
      * Cleares the current goal. Navigation will continue with the next waypoint.
      */
    void clearGoal();

    /**
      * Cleares the current goal and the remaining waypoints in the path.
      */
    void clearPath();

    void pause();
    void resume();

    /**
      * Stops any currently active navigation; clears the history.
      */
    void reset();
    void terminate();

    void addWaypoint( QString name, Waypoint waypoint );
    void removeWaypoint( QString name );
    void saveToSettings();
    void loadFromSettings();

    void xsensUpdate(RobotModule *);
    void sonarPositionUpdate();
    void forwardDone();

signals:
    void healthStatusChanged(HealthStatus data);

    void reachedWaypoint( QString waypoint );
    void failedToReachWayPoint( QString waypoint );
    void updatedWaypoints( QMap<QString, Waypoint> waypoints );
    void setNewGoal( Waypoint goal );
    void clearedGoal();

    void newDepth(float depth);
    void newFFSpeed(float speed);
    void newANGSpeed(float speed);

protected:
    virtual void doHealthCheck();

private:    
    Position defaultPos;

    Module_SonarLocalization *sonarLoc;
    Module_ThrusterControlLoop* tcl;
    Module_PressureSensor *pressure;
    Module_XsensMTi *mti;

    QMap<QDateTime, QString> history;
    QMap<QString, Waypoint> waypoints;

    bool hasActiveGoal;
    QList<QString> path;
    QString currentGoalName;
    Waypoint currentGoal;

    double headingToGoal;
    double distanceToGoal;

    // For driving straight.
    float initialXsensHeading;

    // For not turning too much.
    double adjustHeadingInitialXsens;
    float diffHeading;

    QString state;
    QString substate;
    QString pauseState;
    QString pauseSubstate;

    Position currentPosition;

};
#endif // MODULE_NAVIGATION_H
