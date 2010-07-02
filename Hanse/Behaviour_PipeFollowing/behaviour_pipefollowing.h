#ifndef PIPEBEHAVIOUR_H
#define PIPEBEHAVIOUR_H

#include <Framework/robotbehaviour.h>

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <videoInput.h>
#include <Module_Webcams/module_webcams.h>

#include <Behaviour_PipeFollowing/pipefollowingform.h>

using namespace cv;

class Module_ThrusterControlLoop;
class PipeFollowingForm;

class Behaviour_PipeFollowing : public RobotBehaviour
{
    Q_OBJECT
public:
    Behaviour_PipeFollowing(QString id, Module_ThrusterControlLoop* tcl, Module_Webcams* cam );
    QList<RobotModule*> getDependencies();

    QWidget* createView(QWidget *parent);

    /** starts PipeFollow Behaviour */
    void start();
    /** stops PipeFollow Behaviour */
    void stop();

    void reset();
    /** returns true if Behaviour is active
        return false if the Behaviour is not active
    */
    bool isActive();
    /** Extrahiert die Frames aus einem Videofile und uebergibt sie an findPipe*/
    void analyzeVideo(QString videoFile);
    /** Such ein Rohr in einem Frame und berechnet zugehoerige Parameter */
    void findPipe(Mat &frame, Mat &binaryFrame);
    /** Zeichnet eine Linie aus gegebenen Hough Parametern in ein Bild
        Mat frame - Das Bild
        double rho - Parameter der Gerade
        double theta - Parameter der Gerade
        Scalar color - Farbe der Linie
     */
    void drawLineHough(Mat &frame, double rho, double theta, Scalar color);
    /** sets threshold for segmentation */
    void setThresh(int thresh);
    /** turns debuggin on and off */
    void setDebug(int debug);
    /** sets the ID of the camera to use */
    void setCameraID(int camID);
    /** sets parameter for the p-controller */
    void setDeltaPipe(float deltaDistPipe, float deltaAngPipe);
    /** sets gain for p-controller */
    void setKpDist(float kp);
    void setKpAngle(float kp);
    void setRobCenter(double robCenterX, double robCenterY);
    /** Berechnet den Schnittpunkt der Ideallinie mit einer gefundenen
        Linie (rho, theta) und ermittelt den Abstand zwischen robCenter
        und der gefundenen Gerade.
      */
    void compIntersect(double rho, double theta);
    void compIntersect(Point pt1, Point pt2);
    /** the p-controller. controls the angle speed of the robot */
    void controlPipeFollow();
    /** reset first run for median filter */
    void resetFirstRun();
    /** updates local variables */
    void updateFromSettings();
private:

    /** Update data on data panel */
    void updateData();
    /** uses Hough Transform to find pipe in binary Image (binaryFrame)
      Mat &frame - Picture from the Camera. Used for visual debugging
      Mat &binaryFrame - binary Image for Hough Transform
     */
    void computeLineBinary(Mat &frame, Mat &binaryFrame);
    /** Median Filter
        Eingabewerte werden gefiltert und mit gefilterten ueberschrieben */
    void medianFilter(float &rho, float &theta);
    /** grabs frame from camera device */
    void grab(Mat &frame);

    void moments(Mat &frame);

    void countPixel(Mat &frame, int &sum);

    void initPictureFolder();


//    double m10;
//    double m01;
//    double mu11;
//    double mu02;
//    double mu20;

    Module_ThrusterControlLoop* tcl;
    Module_Webcams* cam;
    QTimer timer;
    QStringList files;
    int fileIndex;

    VideoCapture vc;
    /* konstante parameter */
    int cameraID;
    int threshSegmentation;
    int debug;
    Point robCenter;
    float constFWSpeed;
    float deltaDistPipe;
    float deltaAngPipe;
    float kpDist;
    float kpAngle;
    float maxDistance;
    cv::Mat frame;
    cv::Mat displayFrame;


    /* dynamische parameter */
    float potentialVec;
    float potentialY;
    float distanceY;
    float curAngle;
    Point intersect;
    /* fuer den median */
    float meanRho[5];
    float meanTheta[5];
    int firstRun;
    /* sonstige */
    int noPipeCnt;

private slots:
    void timerSlot();

signals:
    void printFrameOnUi(cv::Mat &frame);

};


#endif // PIPEBEHAVIOUR_H
