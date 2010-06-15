#ifndef VISUALSLAMPARTICLE_H
#define VISUALSLAMPARTICLE_H

#include <QtCore>
#include <QGraphicsScene>
#include <opencv/cxcore.h>
#include <vector>
#include <Module_VisualSLAM/slam/landmark.h>
#include <Module_VisualSLAM/slam/quaternion.h>

using namespace std;

class VisualSLAMParticle
{
public:
    VisualSLAMParticle();
    VisualSLAMParticle( const VisualSLAMParticle *particle );
    ~VisualSLAMParticle();
    double update( vector<CvScalar> pos3D, vector<CvPoint> matches, bool *found );
    void plot( QGraphicsScene *scene );
    void save( QTextStream &ts );
    void load( QTextStream &ts, int landmarkCount );

private:
    bool updatePosition( vector<CvScalar> newPositions, vector<CvPoint> matches );
    double updateMap( vector<CvScalar> newPositions, bool *found, vector<CvPoint> mapMatches );

    void errorGreaterT( vector<CvScalar> newPositions, vector<CvPoint> matches, double T, bool *check, int &count );
    void calcPosition( vector<CvScalar> newPositions, vector<CvPoint> matches, bool *selected, int num );
    void drawRandomSamples( int num, bool *selected );

    void getObservationJacobian( CvMat *Gobservation );
    void getStateJacobian( int landmarkNr, CvMat *Gstate );

public:
    vector<Landmark *> landmarks;

    Quaternion currentRotation;
    CvMat *currentTranslation;
    Quaternion lastRotation;
    CvMat *lastTranslation;

    bool inited;
    double confidence;

private:
    CvMat *Robservation;
    CvMat *Rstate;
    CvMat *Gobservation;
    CvMat *Gstate;

    CvMat *L;
    CvMat *X;

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

#endif // VISUALSLAMPARTICLE_H
