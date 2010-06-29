#ifndef MODULE_VISUALSLAM_H
#define MODULE_VISUALSLAM_H

#include <Framework/robotmodule.h>
#include <Framework/position.h>
#include <Framework/eventthread.h>

#include <QTimer>
#include <QGraphicsScene>

#include <Module_VisualSLAM/capture/stereocapture.h>
#include <Module_VisualSLAM/feature/feature.h>
#include <Module_VisualSLAM/slam/naiveslam.h>

#include <vector>

#define VSLAM_CAMERA_LEFT   0
#define VSLAM_CAMERA_RIGHT  1

using namespace std;

class Module_SonarLocalization;

class Module_VisualSLAM : public RobotModule {
    Q_OBJECT

public:
    Module_VisualSLAM(QString id, Module_SonarLocalization* sonarLocalization);
    ~Module_VisualSLAM();

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
      * Returns the position of an object and the time it was last
      * seen.
      */
    void getObjectPosition( int classNr, QRectF &boundingBox, QDateTime &lastSeen );

    void save( QTextStream &ts );
    void load( QTextStream &ts );

    void getPlotData( QList<Position> &landmarkPositions, QList<Position> &particlePositions, int &bestParticle );
    double getObservationVariance();
    double getTranslationVariance();
    double getRotationVariance();

private:
    void start();
    void stop();

public slots:
    void statusChange( bool value );
    void setEnabled( bool value );
    void reset();
    void terminate();

    void startGrab();
    void startUpdate();
    void finishUpdate();
    void changeSettings( double v_observation, double v_translation, double v_rotation );
    void updateSonarData();
    void updateObject( int classNr );

    void test();

signals:
    void healthStatusChanged(HealthStatus data);

    void newLocalizationEstimate();
    void lostLocalization();
    void updateFinished();
    void viewUpdated();
    void foundNewObject( int classNr );
    void timerStart( int msec );
    void timerStop();

private:
    Module_SonarLocalization* sonarLocalization;
    //QTimer updateTimer;
    EventThread updateThread;
    QDateTime lastRefreshTime;
    clock_t startClock;
    clock_t stopClock;

    StereoCapture cap;
    Feature feature;
    NaiveSLAM slam;
    bool enabled;

    QTimer testTimer;
};

#endif // MODULE_VISUALSLAM_H
