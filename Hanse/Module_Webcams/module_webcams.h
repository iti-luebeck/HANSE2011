#ifndef MODULE_WEBCAMS_H
#define MODULE_WEBCAMS_H

#include <QtGui>
#include <QtCore>
#include <Framework/robotmodule.h>
//#include <videoInput.h>
#include <opencv/cxcore.h>
#include <opencv/highgui.h>
#include <opencv/cv.h>

#define WEBCAM_WIDTH    640
#define WEBCAM_HEIGHT   480

class Module_Webcams : public RobotModule
{
    Q_OBJECT
public:
    Module_Webcams( QString id );
    ~Module_Webcams();
    QWidget* createView(QWidget* parent);
    QList<RobotModule*> getDependencies();


    void grabLeft( IplImage *left );
    void grabRight( IplImage *right );
    void grabBottom( IplImage *bottom );
    void grabLeft( cv::Mat &left );
    void grabRight( cv::Mat &right );
    void grabBottom( cv::Mat &bottom );
    int numOfCams();

private:
    void stopWebcams();
    void init();

public slots:

    void statusChange( bool value );
    void settingsChanged();
    void reset();
    void terminate();
    void showSettings( int camNr );

private:
//    videoInput VI;

//    cv::VideoCapture* leftCap;
//    cv::VideoCapture* rightCap;
//    cv::VideoCapture* bottomCap;

    CvCapture* leftCap;
    CvCapture* rightCap;
    CvCapture* bottomCap;

    int nCams;
    int leftID;
    bool leftConnected;
    bool leftEnabled;
    int leftFramerate;
    int rightID;
    bool rightConnected;
    bool rightEnabled;
    int rightFramerate;
    int bottomID;
    bool bottomConnected;
    bool bottomEnabled;
    int bottomFramerate;
    QMutex mutex;

    IplImage *leftFrame;
    IplImage *rightFrame;
    IplImage *bottomFrame;
    int count;
};

#endif // MODULE_WEBCAMS_H
