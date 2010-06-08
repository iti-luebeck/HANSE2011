#ifndef PIPEBEHAVIOUR_H
#define PIPEBEHAVIOUR_H

#include <Framework/robotbehaviour.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>

using namespace cv;

class Module_ThrusterControlLoop;

class Behaviour_PipeFollowing : public RobotBehaviour
{
    Q_OBJECT
public:
    Behaviour_PipeFollowing(QString id, Module_ThrusterControlLoop* tcl);

    QList<RobotModule*> getDependencies();

    QWidget* createView(QWidget *parent);

    void start();

    void stop();

    bool isActive();
    /** Extrahiert die Frames aus einem Videofile und uebergibt sie an findPipe*/
    void analyzeVideo(QString videoFile);
    /** Such ein Rohr in einem Frame und berechnet zugehoerige Parameter */
    void findPipe(Mat &frame);
    /** Zeichnet eine Linie aus gegebenen Hough Parametern in ein Bild
        Mat frame - Das Bild
        double rho - Parameter der Gerade
        double theta - Parameter der Gerade
        Scalar color - Farbe der Linie
     */
    void drawLineHough(Mat &frame, double rho, double theta, Scalar color);
    void setThresh(int thresh);
    void setDebug(int debug);
    void setCameraID(int camID);
    void setDeltaPipe(double deltaPipe);
    void setSpeed(double speed);
    void setKp(double kp);
    void setRobCenter(double robCenterX, double robCenterY);
    /** Berechnet den Schnittpunkt der Ideallinie mit einer gefundenen
        Linie (rho, theta) und ermittelt den Abstand zwischen robCenter
        und der gefundenen Gerade.
      */
    void compIntersect(double rho, double theta);
    /** Der regler */
    void controlPipeFollow();

private:

    /** Update data on data panel */
    void updateData();
    Module_ThrusterControlLoop* tcl;
    QTimer timer;
    bool active;
    VideoCapture vc;
    int cameraID;
    int i;
    int threshSegmentation;
    int debug;

    double distance;
    float curAngle;
    Point intersect;
    int potentialVec;
    Point robCenter;

    double deltaDistPipe;
    double deltaAngPipe;
    double kp;
    double speed;


private slots:
    void timerSlot();


};


#endif // PIPEBEHAVIOUR_H
