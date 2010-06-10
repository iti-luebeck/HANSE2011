#ifndef STEREOCAPTURE_H
#define STEREOCAPTURE_H

#include <QRectF>
#include <QDateTime>
#include <QMutex>
#include <opencv/cxcore.h>
#include <vector>
#include "videoInput.h"
#include <Module_VisualSLAM/feature/feature.h>

#define GOAL_LABEL  1
#define BALL_LABEL  2

class Module_VisualSLAM;

using namespace std;

class StereoCapture : public QObject
{
    Q_OBJECT

public:
    StereoCapture( int width = 640, int height = 480, int device1 = 0, int device2 = 1 );
    ~StereoCapture();
    void grab();
    bool isConnected( int device );
    IplImage *getFrame( int cam );
    void getObjectPosition( int classNr, QRectF &boundingBox, QDateTime &lastSeen );
    void setMutex( QMutex *mutex );

public slots:
    void surfDone1();
    void surfDone2();

private:
    void initStereoCalibration();
    void normalizePixels( CvMat *x, CvMat *xn, double f, double cm, double cn);
    void stereoTriangulation(CvMat *xl, CvMat *xr, vector<CvScalar> &X, double T);
    void doCalculations();

private:
    clock_t start, stop;

    int width;
    int height;
    int count;

    videoInput VI;
    int device1;
    int device2;

    bool connected1;
    bool connected2;

    CvMat *Q;
    CvMat *mapX_left;
    CvMat *mapY_left;
    CvMat *mapX_right;
    CvMat *mapY_right;

    double f;
    double cn;
    double cm;
    double newT;

    Feature feature1;
    Feature feature2;
    bool done1;
    bool done2;

    vector<CvScalar> keypoints1;
    vector<CvScalar> keypoints2;
    IplImage *frame1;
    IplImage *frame2;
    vector<CvMat *> *descriptors;
    vector<CvScalar> *pos2D;
    vector<CvScalar> *pos3D;
    vector<int> *classesVector;

    vector<CvMat *> classFeatures;
    vector<int> classLabels;
    vector<QRectF> classRects;
    vector<QDateTime> classLastSeen;

    QMutex *grabMutex;

signals:
    void grabFinished( vector<CvMat *> descriptors, vector<CvScalar> pos2D, vector<CvScalar> pos3D, vector<int> classesVector );
};

#endif // STEREOCAPTURE_H
