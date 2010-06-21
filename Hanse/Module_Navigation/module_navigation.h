#ifndef MODULE_NAVIGATION_H
#define MODULE_NAVIGATION_H

#include <QtCore>
#include <QGraphicsScene>
#include <Framework/robotmodule.h>
#include <Framework/position.h>

class Module_Localization;
class Module_ThrusterControlLoop;

class Module_Navigation : public RobotModule
{
    Q_OBJECT
public:
    Module_Navigation(QString id, Module_Localization *localization, Module_ThrusterControlLoop* tcl);

    QWidget* createView(QWidget* parent);

    QList<RobotModule*> getDependencies();

    /**
      * get the list of all waypoints.
      */
    QStringList getWayPoints();

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

    /**
      * move the robot the given waypoint.
      * stop if the robot is within "delta" range of the waypoint.
      *
      * if delta is ommited, a default range will be used.
      */
    void gotoWayPoint(QString name, Position delta);

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

public slots:

    /**
      * Stops any currently active navigation; clears the history.
      */
    void reset();
    void terminate();
    void addWaypoint( QString name, Position pos );
    void removeWaypoint( QString name );
    void save( QString path );
    void load( QString path );

signals:
    void healthStatusChanged(HealthStatus data);

    void reachedWaypoint(QString waypoint);
    void failedToReachWayPoint(QString waypoint);
    void updatedWaypoints( QMap<QString, Position> waypoints );

protected:
    virtual void doHealthCheck();

private:    
    Position defaultPos;
    Module_Localization *localization;
    Module_ThrusterControlLoop* tcl;
    QMap<QDateTime, QString> history;
    QMap<QString, Position> waypoints;

};
#endif // MODULE_NAVIGATION_H
