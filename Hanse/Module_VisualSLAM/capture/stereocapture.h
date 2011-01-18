#ifndef STEREOCAPTURE_H
#define STEREOCAPTURE_H

#include <QRectF>
#include <QDateTime>
#include <QMutex>
#include <opencv/cxcore.h>
#include <vector>
//#include "videoInput.h"
#include <Module_VisualSLAM/feature/feature.h>

#define GOAL_LABEL  1
#define BALL_LABEL  2

class Module_VisualSLAM;
class Module_Webcams;

using namespace std;

class StereoCapture : public QObject
{
    Q_OBJECT

public:
    StereoCapture( Module_Webcams *cams );
    ~StereoCapture();
    void init();
    void grab( bool saveImages, bool noSLAM );
    IplImage *getFrame( int cam );
    void getObjectPosition( int classNr, QRectF &boundingBox, QDateTime &lastSeen );

    vector<CvMat *> *getDescriptors();
    vector<CvScalar> *getPos();
    vector<int> *getClasses();

private:
    void clear();
    void initStereoCalibration();
    void normalizePixels( CvMat *x, double f, double cm, double cn);
    void stereoTriangulation(CvMat *xl, CvMat *xr, vector<CvScalar> *X, double T);
    void doCalculations();

private:
    clock_t start, stop;

    Module_Webcams *cams;
    int count;

    CvMat *Q;
    CvMat *mapX_left;
    CvMat *mapY_left;
    CvMat *mapX_right;
    CvMat *mapY_right;

    double f;
    double cn;
    double cm;
    double newT;

    Feature *feature1;
    Feature *feature2;
    bool done1;
    bool done2;

    vector<CvScalar> keypoints1;
    vector<CvScalar> keypoints2;
    IplImage *frame1;
    IplImage *frame1_gray;
    IplImage *frame2;
    IplImage *frame2_gray;
    vector<CvMat *> *descriptors;
    vector<CvScalar> *pos;
    vector<int> *classes;

    vector<CvMat *> classFeatures;
    vector<int> classLabels;
    vector<QRectF> classRects;
    vector<QDateTime> classLastSeen;

    QMutex captureMutex;

signals:
    void grabFinished();
    void foundNewObject( int classNr );
};

#endif // STEREOCAPTURE_H
