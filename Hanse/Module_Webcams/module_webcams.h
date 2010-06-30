#ifndef MODULE_WEBCAMS_H
#define MODULE_WEBCAMS_H

#include <QtGui>
#include <QtCore>
#include <Framework/robotmodule.h>
#include <videoInput.h>
#include <opencv/cxcore.h>

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
    void grabBottom( cv::Mat &bottom );

private:
    void stopWebcams();

public slots:
    void statusChange( bool value );
    void settingsChanged();
    void reset();
    void terminate();

private:
    videoInput VI;
    int nCams;
    int leftID;
    bool leftConnected;
    int rightID;
    bool rightConnected;
    int bottomID;
    bool bottomConnected;
    QMutex mutex;
};

#endif // MODULE_WEBCAMS_H
