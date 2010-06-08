#ifndef LANDMARK_H
#define LANDMARK_H

#include <opencv/cxcore.h>

class Landmark
{
public:
    Landmark( CvMat *observation, CvMat *Robservation, int featureNr, int classNr );
    ~Landmark();
    double update( CvMat *observation, CvMat *expectedObservation,
                   CvMat *Robservation, CvMat *Rstate,
                   CvMat *Gobservation, CvMat *Gstate );
    void addReference();
    int removeReference();
    CvMat *getPos();
    double getPos( int i );
    CvMat *getSigma();
    int getClass();

private:
    CvMat *Sigma;
    CvMat *pos;
    CvMat *Z;
    CvMat *Zinv;
    CvMat *K;
    CvMat *diffPos;
    CvMat *I3;
    CvMat *Mtemp1;
    CvMat *Mtemp2;
    CvMat *Mtemp3;
    CvMat *Mtemp4;
    CvMat *L;
    int references;
    int featureNr;
    int classNr;
};

#endif // LANDMARK_H
