#ifndef MODULE_LOCALIZATION_H
#define MODULE_LOCALIZATION_H

#include <Framework/robotmodule.h>
#include <Framework/position.h>

class Module_VisualSLAM;
class Module_SonarLocalization;
class Module_PressureSensor;

/**
  * TODO: map
  */
class Module_Localization : public RobotModule
{
    Q_OBJECT

public:

    Module_Localization(QString id, Module_VisualSLAM* visualSLAM, Module_SonarLocalization *sonarLoc,
                        Module_PressureSensor* pressure);

    QWidget* createView(QWidget* parent);

    QList<RobotModule*> getDependencies();

    /**
      * Returns the current position and orientation of the robot.
      *
      */
    Position getLocalization();

    /**
      * How long ago has the position been updated.
      * (returns unix time: ellapsed seconds since 1070)
      */
    QDateTime getLastRefreshTime();

    /**
      * Returns the confidence, by which the Position returned by
      * getLocalization() should be meassured.
      *
      * Range: 0-1 (0==no confidence; 1==full confidence)
      */
    float getLocalizationConfidence();

    /**
      * Returns true if the robot is lost, i.e. the confidence is
      * below a certain threshold.
      */
    bool isLocalizationLost();

    /**
      * Returns the path we have passed until now.
      *
      * key: time
      * value: name of the waypoint
      */
    const QMap<QDateTime, Position> getPath();

public slots:
    /**
      * clears the history.
      */
    void reset();
    void terminate();

signals:
    void healthStatusChanged(HealthStatus data);

    /**
      * is emitted when a new localization update has arrived.
      */
    void newLocalizationEstimate();

    /**
      * Is emitted once this module thinks that it lost the localization.
      *
      * while the localization is lost, no "newLocalizationEstimate" will be emitted.
      *
      */
    void lostLocalization();

protected:
    virtual void doHealthCheck();

private:
    Module_VisualSLAM *visualSLAM;
    Module_SonarLocalization *sonarLocalization;
    Module_PressureSensor *pressure;
    QMap<QDateTime, Position> path;

};

#endif // MODULE_LOCALIZATION_H
