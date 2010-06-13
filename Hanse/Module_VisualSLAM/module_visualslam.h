#ifndef MODULE_VISUALSLAM_H
#define MODULE_VISUALSLAM_H

#include <Framework/robotmodule.h>
#include <Framework/position.h>

#include <QTimer>
#include <QThread>
#include <QGraphicsScene>

#include <vector>

#include <Module_VisualSLAM/capture/stereocapture.h>
#include <Module_VisualSLAM/feature/feature.h>
#include <Module_VisualSLAM/slam/naiveslam.h>

using namespace std;

class Module_SonarLocalization;
class Form_VisualSLAM;

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

    /**
      * Returns the position of an object and the time it was last
      * seen.
      */
    void getObjectPosition( int classNr, QRectF &boundingBox, QDateTime &lastSeen );

    void plot( QGraphicsScene *scene );

    void save( QTextStream &ts );

    void run();
    void start();
    void stop();

    QMutex *getSceneMutex();
    QMutex *getUpdateMutex();

public slots:
    void reset();
    void terminate();
    void update();
    void updateMap( vector<CvMat *> descriptors, vector<CvScalar> pos2D, vector<CvScalar> pos3D, vector<int> classesVector );

signals:
    void healthStatusChanged(HealthStatus data);

    void newLocalizationEstimate();
    void lostLocalization();
    void updateFinished();

private:
    Module_SonarLocalization* sonarLocalization;
    QTimer updateTimer;
    QDateTime lastRefreshTime;
    clock_t startClock;
    clock_t stopClock;
    QMutex sceneMutex;
    QMutex updateMutex;

    QGraphicsScene *scene;
    StereoCapture cap;
    Feature feature;
    NaiveSLAM slam;
    bool stopped;
};

#endif // MODULE_VISUALSLAM_H
