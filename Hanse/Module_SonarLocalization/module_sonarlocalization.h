#ifndef MODULE_SONARLOCALIZATION_H
#define MODULE_SONARLOCALIZATION_H

#include <Framework/robotmodule.h>
#include <Framework/position.h>
#include <Module_ScanningSonar/sonarreturndata.h>
#include <opencv/cv.h>

class Module_ScanningSonar;
class SonarEchoFilter;
class SonarParticleFilter;

class Module_SonarLocalization : public RobotModule {
    Q_OBJECT

    friend class Form_SonarLocalization;

public:
    Module_SonarLocalization(QString id, Module_ScanningSonar* sonar);

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

public slots:
    void reset();
    void terminate();

signals:
    void healthStatusChanged(HealthStatus data);

    void newLocalizationEstimate();
    void lostLocalization();

private:
    Module_ScanningSonar* sonar;
    SonarEchoFilter* filter;
    SonarParticleFilter* pf;

};

#endif // MODULE_SONARLOCALIZATION_H
