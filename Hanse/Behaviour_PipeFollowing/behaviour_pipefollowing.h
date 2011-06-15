#ifndef PIPEBEHAVIOUR_H
#define PIPEBEHAVIOUR_H

#include <Framework/robotbehaviour.h>

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <Module_Webcams/module_webcams.h>
#include <Behaviour_PipeFollowing/pipefollowingform.h>

using namespace cv;

class Module_ThrusterControlLoop;
class PipeFollowingForm;
class Module_Simulation;

class Behaviour_PipeFollowing : public RobotBehaviour
{
    Q_OBJECT
public:
    Behaviour_PipeFollowing(QString id, Module_ThrusterControlLoop* tcl, Module_Webcams* cam, Module_Simulation *sim );
    QList<RobotModule*> getDependencies();

    QWidget* createView(QWidget *parent);

    /** returns true if Behaviour is active
        return false if the Behaviour is not active
    */
    bool isActive();

    /**
      * gibt den aktuellen Frame zurueck
      */
    void grabFrame(cv::Mat &frame);

private:
    void init();

    /** Such ein Rohr in einem Frame und berechnet zugehoerige Parameter */
    void findPipe(Mat &frame, Mat &binaryFrame);

    /** Berechnet den Schnittpunkt der Ideallinie mit einer gefundenen
        Linie (rho, theta) und ermittelt den Abstand zwischen robCenter
        und der gefundenen Gerade.
      */
    void compIntersect(Point pt1, Point pt2);

    /** the p-controller. controls the angle speed of the robot */
    void controlPipeFollow();

    /** Update data on data panel */
    void updateData();

    /** use cv moments to compute pipe */
    void moments(Mat &frame);

    /**
     * converts frame to selected color space
      0 for hsv
      1 for h
      2 for s
      3 for v
      4 for gray
      as selected in settings pane
    */
    void convertColor(Mat &frame, Mat &convFrame);

    /** computation */
    void timerSlotExecute();

    Module_ThrusterControlLoop* tcl;
    Module_Webcams* cam;
    Module_Simulation *sim;
    QTimer timer;
    QStringList files;
    int fileIndex;

    /* konstante parameter */
    int threshSegmentation;
    int debug;
    int timerTime;
    Point robCenter;
    float constFWSpeed;
    float deltaDistPipe;
    float deltaAngPipe;
    float kpDist;
    float kpAngle;
    float maxDistance;
    cv::Mat frame;
    cv::Mat displayFrame;
    cv::Mat segmentationFrame;


    /* dynamische parameter */
    float potentialVec;
    float potentialY;
    float distanceY;
    float curAngle;
    Point intersect;
    int firstRun;
    /* sonstige */
    int noPipeCnt;
    int toSlowCnt;

private slots:
    void timerSlot();

public slots:
    /** updates local variables */
    void updateFromSettings();
    void startBehaviour();
    void stop();
    void reset();
    void terminate();
    void simFrame(cv::Mat simFrame);
    void setUpdatePixmapSlot(bool bol);
    void controlEnabledChanged(bool);

    /** Extrahiert die Frames aus einem Videofile und uebergibt sie an findPipe*/
    void analyzeVideo();

signals:
//    void printFrameOnUi(cv::Mat &frame);
    void timerStart( int msec );
    void timerStop();

    void forwardSpeed(float fwSpeed);
    void angularSpeed(float anSpeed);
    void requestBottomFrame();
    void setUpdatePixmapSignal(bool bol);

};


#endif // PIPEBEHAVIOUR_H
