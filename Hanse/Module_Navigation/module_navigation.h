#ifndef MODULE_NAVIGATION_H
#define MODULE_NAVIGATION_H

#include <QtCore>
#include <QGraphicsScene>
#include <Framework/robotmodule.h>
#include <Framework/position.h>

#define NAV_STATE_IDLE                  "idle"
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
class Module_Compass;
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
                       Module_Compass *compass,
                       Module_XsensMTi *mti );

    QWidget* createView(QWidget* parent);

    QList<RobotModule*> getDependencies();

    /**
      * get the list of all waypoints.
      */
    QMap<QString, Position> getWaypoints();

    /**
      * get the position of a waypoint
      */
    Position getWayPointPosition(QString name);

    /**
      * add a new waypoint
      *
      * position: the position of the waypoint, in map coordinates
      * name: the name of the waypoint. the name must be unique
      *       and must not already been used.
      *
      * if you omit a specific name, a random one will be chosen.
      * and returned.
      */
    QString addWayPoint(Position position,QString name="");


    void clearGoal();

    /**
      * Returns the current destination or an empty String (""),
      * when we are at rest.
      */
    QString currentDestination();

    /**
      * returns true if the robot is currently trying to reach a waypoint.
      */
    bool isCurrentlyMoving();

    /**
      * Returns the list of all successfully passed waypoints,
      * sorted by time in ascending order.
      *
      * key: time, in seconds
      * value: name of the waypoint
      */
    const QMap<QDateTime, QString> getWayPointHistory();

private:
    void saveWaypoints( QTextStream &ts );
    void loadWaypoints( QTextStream &ts );
    void init();
    float pi2pi(float ang);
    float ang2ang(float ang);

public slots:
    /**
      * move the robot the given waypoint.
      * stop if the robot is within "delta" range of the waypoint.
      *
      * if delta is ommited, a default range will be used.
      */
    void gotoWayPoint(QString name, Position delta);

    /**
      * Stops any currently active navigation; clears the history.
      */
    void reset();
    void terminate();
    void addWaypoint( QString name, Position pos );
    void removeWaypoint( QString name );
    void save( QString path );
    void load( QString path );

    void compassUpdate(RobotModule *);
    void xsensUpdate(RobotModule *);
    void sonarPositionUpdate();
    void forwardDone();

signals:
    void healthStatusChanged(HealthStatus data);

    void reachedWaypoint( QString waypoint );
    void failedToReachWayPoint( QString waypoint );
    void updatedWaypoints( QMap<QString, Position> waypoints );
    void setNewGoal( Position goal );
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
    Module_Compass *compass;
    Module_XsensMTi *mti;

    QMap<QDateTime, QString> history;
    QMap<QString, Position> waypoints;

    QString currentGoalName;
    Position currentGoalPosition;

    double headingToGoal;
    double distanceToGoal;
    float initialCompassHeading;
    float initialXsensHeading;

    QString state;
    QString substate;

};
#endif // MODULE_NAVIGATION_H
