#ifndef MODULE_VISUALSLAM_H
#define MODULE_VISUALSLAM_H

#include <Framework/robotmodule.h>
#include <Framework/position.h>

class Module_SonarLocalization;

class Module_VisualSLAM : public RobotModule {
    Q_OBJECT

public:
    Module_VisualSLAM(QString id, Module_SonarLocalization* sonarLocalization);

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
    Module_SonarLocalization* sonarLocalization;
};

#endif // MODULE_VISUALSLAM_H
