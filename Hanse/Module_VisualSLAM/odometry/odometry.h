#ifndef ODOMETRY_H
#define ODOMETRY_H

#include <vector>
#include <opencv/cxcore.h>

using namespace std;

class Odometry
{
public:
    Odometry();
    ~Odometry();
    void getOdometry( CvMat *descriptors, vector<CvScalar> newPositions2D, vector<CvScalar> newPositions3D, CvMat *R, CvMat *T );

private:
    void calcFivePoint( vector<CvScalar> last, vector<CvScalar> next, vector<CvMat *> &Es );
    void estimateTranslation( CvMat *R, CvMat *T, vector<CvScalar> last, vector<CvScalar> next, double &scale );
    void drawRandomSamples( int numSamples, int numTotal, bool *selected );
    void fivePointHelper( CvMat *EE, CvMat *A );
    int calcConsensusSet( vector<CvScalar> last, vector<CvScalar> next, vector<CvMat *> Es, CvMat *E );
    void getCameraMatrices( CvMat *E, vector<CvMat *> &Rs, vector<CvMat *> &Ts );
    void getCorrectCameraMatrix( vector<CvMat *> &Rs, vector<CvMat *> &Ts, CvMat *Rcorrect,
                                 CvMat *Tcorrect, CvScalar x1, CvScalar x2 );

private:
    vector<CvScalar> last2DPositions;
    vector<CvScalar> last3DPositions;
    CvMat *lastDescriptors;
};

#endif // ODOMETRY_H
