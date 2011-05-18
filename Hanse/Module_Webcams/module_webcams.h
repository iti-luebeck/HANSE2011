#ifndef MODULE_WEBCAMS_H
#define MODULE_WEBCAMS_H

#include <QtGui>
#include <QtCore>
#include <Framework/robotmodule.h>
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


    /**
      * Grab current Frame from Webcam
      * @param left / right / bottom - cv::Mat or IplImage to store
      */
    void grabLeft( IplImage *left );
    void grabRight( IplImage *right );
    void grabBottom( IplImage *bottom );
    void grabLeft( cv::Mat &left );
    void grabRight( cv::Mat &right );
    void grabBottom( cv::Mat &bottom );

    /**
      * Returns Vector containing Indices of available Cams
      * @return Vector containing Indices of available Cams
      */
    std::vector<int> numOfCams();

public slots:

    void settingsChanged();
    void reset();
    void terminate();

private:

     void init();
    /**
      * disconnects all connected devices
      */
    void stopWebcams();

    /**
      * Returns number of available video devices
      * @return int - number of cams available
      */
    int numAvailableCams();


    CvCapture* leftCap;
    CvCapture* rightCap;
    CvCapture* bottomCap;

    std::vector<int> camInd;
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

    IplImage *leftFrame;
    IplImage *rightFrame;
    IplImage *bottomFrame;
};

#endif // MODULE_WEBCAMS_H
