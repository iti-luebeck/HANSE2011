#ifndef NAIVESLAM_H
#define NAIVESLAM_H

#include <QGraphicsScene>
#include <opencv/cxcore.h>
#include <vector>
#include "landmark.h"
#include "quaternion.h"
#include <Framework/position.h>

using namespace std;

class NaiveSLAM
{
public:
    NaiveSLAM();
    ~NaiveSLAM();
    bool update( vector<CvMat *> descriptor, vector<CvScalar> &pos3D, vector<CvScalar> &pos2D, vector<int> classLabels );
    void plot( QGraphicsScene *scene );
    void save( const char *fileName );
    void load( const char *fileName );
    Position getPosition();
    double getConfidence();

    void test();
    void reset();

private:
    bool updatePosition( vector<CvScalar> newPositions, vector<CvPoint> matches );
    void updateMap( vector<CvMat *> newFeatures, vector<CvScalar> newPositions,
                    bool *found, vector<CvPoint> mapMatches, vector<int> classLabels );

    void errorGreaterT( vector<CvScalar> newPositions, vector<CvPoint> matches, double T, bool *check, int &count );
    void calcPosition( vector<CvScalar> newPositions, vector<CvPoint> matches, bool *selected, int num );
    void drawRandomSamples( int num, bool *selected );

    void getObservationJacobian( CvMat *Gobservation );
    void getStateJacobian( int landmarkNr, CvMat *Gstate );

private:
    CvMemStorage *storage;

    bool inited;

    CvSeq *features;
    vector<Landmark *> landmarks;

    CvMat *Robservation;
    CvMat *Rstate;
    CvMat *Gobservation;
    CvMat *Gstate;

    Quaternion currentRotation;
    CvMat *currentTranslation;

    Quaternion lastRotation;
    CvMat *lastTranslation;

    Position pos;
    double confidence;

    // Temporary variables for position estimation.
    CvMat *P3;
    CvMat *Q3;
    CvMat *K;
    CvMat *R;
    CvMat *U;
    CvMat *D;
    CvMat *V;
    CvMat *meanLocalPosition;
    CvMat *meanGlobalPosition;
};

#endif // NAIVESLAM_H
