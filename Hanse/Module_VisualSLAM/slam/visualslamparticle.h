#ifndef VISUALSLAMPARTICLE_H
#define VISUALSLAMPARTICLE_H

#include <QtCore>
#include <QGraphicsScene>
#include <opencv/cxcore.h>
#include <vector>
#include <Module_VisualSLAM/slam/quaternion.h>

using namespace std;

#define DEFAULT_OBSERVATION_VARIANCE    0.09
#define DEFAULT_TRANSLATION_VARIANCE    0.01
#define DEFAULT_ROTATION_VARIANCE       0.0025
#define RANSAC_THRESHOLD                0.2
#define P0                              0.01

class Position;
class Landmark;

class VisualSLAMParticle
{
public:
    VisualSLAMParticle();
    VisualSLAMParticle( const VisualSLAMParticle *particle );
    ~VisualSLAMParticle();

    double update( vector<CvScalar> *pos, vector<CvPoint> *matches, bool *found );

    void setObservationVariance( double v );
    void setTranslationVariance( double v );
    void setRotationVariance( double v );

    void plot( QGraphicsScene *scene );
    void save( QTextStream &ts );
    void load( QTextStream &ts, int landmarkCount );

    Position getLandmarkPosition( int i );
    Position getParticlePosition();

private:
    bool updatePosition( vector<CvScalar> *newPositions, vector<CvPoint> *mapMatches );
    double updateMap( vector<CvScalar> *newPositions, bool *found, vector<CvPoint> *mapMatches );

    void errorGreaterT( vector<CvScalar> *newPositions, vector<CvPoint> *matches, double T, bool *check, int &count );
    void calcPosition( vector<CvScalar> *newPositions, vector<CvPoint> *matches, bool *selected, int num );
    void drawRandomSamples( int num, bool *selected );

    void getObservationJacobian( CvMat *Gobservation );
    void getStateJacobian( int landmarkNr, CvMat *Gstate );

public:
    vector<Landmark *> landmarks;
    vector<QGraphicsEllipseItem *> items;

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
