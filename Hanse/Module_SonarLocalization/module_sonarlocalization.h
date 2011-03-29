#ifndef MODULE_SONARLOCALIZATION_H
#define MODULE_SONARLOCALIZATION_H

#include <Framework/robotmodule.h>
#include <Framework/position.h>
#include <Module_ScanningSonar/sonarreturndata.h>
#include <opencv/cv.h>
#include <opencv/ml.h>
#include <Module_SonarLocalization/sonarechofilter.h>
#include <Module_SonarLocalization/sonarparticlefilter.h>

class Module_ScanningSonar;
class Module_PressureSensor;

class Module_SonarLocalization : public RobotModule {
    Q_OBJECT

    friend class Form_SonarLocalization;

public:
    Module_SonarLocalization(QString id, Module_ScanningSonar* sonar, Module_PressureSensor* pressure);

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


    SonarParticleFilter& particleFilter();

public slots:
    void reset();
    void terminate();
    void retrieveSonarEchoData(SonarEchoData data);
    void setLocalization(QVector2D position);

signals:
    void healthStatusChanged(HealthStatus data);

    void newSonarEchoData(SonarEchoData data);
    void newLocalizationEstimate();
    void lostLocalization();

private:
    Module_ScanningSonar* sonar;
    Module_PressureSensor* pressure;

    SonarEchoFilter filter;
    SonarParticleFilter pf;
    CvSVMParams svmParam;
    CvSVM* svm;

    void init();

private slots:
    void newPositionEst(QVector3D p);

};

#endif // MODULE_SONARLOCALIZATION_H
