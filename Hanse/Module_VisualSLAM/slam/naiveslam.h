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

class NaiveSLAM
{
public:
    NaiveSLAM( int particleCount );
    ~NaiveSLAM();
    bool update( vector<CvMat *> descriptor, vector<CvScalar> pos3D, vector<int> classLabels );
    void plot( QGraphicsScene *scene );
    void save( QTextStream &ts );
    void load( QTextStream &ts );
    Position getPosition();
    double getConfidence();

    void test();
    void reset();

private:
    void resampleParticles( double *weights );

private:
    CvMemStorage *storage;
    CvSeq *features;
    vector<int> classes;

    vector<VisualSLAMParticle *> particles;
    int bestParticle;

    Position pos;
    double confidence;
};

#endif // NAIVESLAM_H
