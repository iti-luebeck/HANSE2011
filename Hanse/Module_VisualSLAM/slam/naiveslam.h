#ifndef NAIVESLAM_H
#define NAIVESLAM_H

#include <QGraphicsScene>
#include <QTextStream>
#include <opencv/cxcore.h>
#include <vector>
#include "landmark.h"
#include "quaternion.h"
#include <Framework/position.h>
#include <Module_VisualSLAM/slam/visualslamparticle.h>

using namespace std;

class NaiveSLAM : public QThread
{
    Q_OBJECT
public:
    NaiveSLAM( int particleCount );
    ~NaiveSLAM();
    void update( vector<CvMat *> *descriptor, vector<CvScalar> *pos3D, vector<int> *classLabels );
    void save( QTextStream &ts );
    void load( QTextStream &ts );
    Position getPosition();
    double getConfidence();
    void setObservationVariance( double v );
    void setTranslationVariance( double v );
    void setRotationVariance( double v );
    void getLandmarkPositions( QList<QPointF> &landmarkPositions );
    void setOffset( Position diffPos );

    void test();
    void reset();

    void run();

private:
    void resampleParticles( double *weights );

signals:
    void updateDone();

private:
    CvMemStorage *storage;
    CvSeq *features;
    vector<int> classes;

    vector<VisualSLAMParticle *> particles;
    int bestParticle;

    Position pos;
    Position offset;
    CvMat *meanTranslation;
    Quaternion meanRotation;
    double confidence;

    vector<CvMat *> *descriptor;
    vector<CvScalar> *pos3D;
    vector<int> *classLabels;

    QMutex updateMutex;
};

#endif // NAIVESLAM_H
