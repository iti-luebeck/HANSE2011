#ifndef MODULE_WEBCAMS_H
#define MODULE_WEBCAMS_H

#include <QtGui>
#include <QtCore>
#include <Framework/robotmodule_mt.h>
#include <videoInput.h>
#include <opencv/cxcore.h>

#define WEBCAM_WIDTH    640
#define WEBCAM_HEIGHT   480

class Module_Webcams : public RobotModule_MT
{
    Q_OBJECT
public:
    Module_Webcams( QString id );
    ~Module_Webcams();
    QWidget* createView(QWidget* parent);
    QList<RobotModule*> getDependencies();



private:
    void stopWebcams();

public slots:

    void grabLeft( IplImage *left );
    void grabRight( IplImage *right );
    void grabBottom( IplImage *bottom );
    void grabBottom( cv::Mat &bottom );

    void statusChange( bool value );
    void settingsChanged();
    void reset();
    void terminate();
    void showSettings( int camNr );

private:
    videoInput VI;
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
