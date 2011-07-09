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
      * Returns Vector containing Indices of available Cams
      * @return Vector containing Indices of available Cams
      */
    std::vector<int> numOfCams();

private:

    void init();
    void stopWebcams();
    int numAvailableCams();

public slots:

    void settingsChanged();
    void reset();
    void terminate();

private slots:

    void grabCams();

signals:
    void newFrontImage(cv::Mat image);
    void newBottomImage(cv::Mat image);

private:

    CvCapture* frontCap;
    CvCapture* bottomCap;

    std::vector<int> camInd;
    int nCams;
    int frontID;
    bool frontEnabled;
    int bottomID;
    bool bottomEnabled;

    IplImage *frontFrame;
    IplImage *bottomFrame;

    QTimer updateTimer;
};

#endif // MODULE_WEBCAMS_H
