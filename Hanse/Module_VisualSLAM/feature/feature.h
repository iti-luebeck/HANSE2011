#ifndef FEATURE_H
#define FEATURE_H

#include <QThread>
#include <opencv/cxcore.h>
#include <vector>
#include "surf/surflib.h"
// #include <opencv2/features2d/features2d.hpp>

using namespace std;

class Feature : public QThread
{
    Q_OBJECT

public:
    Feature();
    ~Feature();
    void run();
    void findFeatures( IplImage *image, CvSeq **keypoints, CvSeq **descriptors );
    void findFeatures( IplImage *image, vector<CvScalar> &keypoints );
    void updateThreshold( int numFeatures );
    static void matchFeatures( CvSeq *descriptors1, CvSeq *descriptors2, vector<CvPoint> &matches );
    static void matchFeatures( CvMat *descriptors1, CvMat *descriptors2, vector<CvPoint> &matches );
    CvMat *matchFeatures( IplImage *image1, IplImage *image2, vector<CvScalar> &keypoints1, vector<CvScalar> &keypoints2 );
    CvMat *getDescriptor();

private:
    void convertToGray(IplImage *rgb, IplImage *gray);

private:
    CvMemStorage *storage;
    CvSURFParams params;
    double surfThreshold;
    double fastThreshold;
//    cv::RTreeClassifier detector;
    IplImage *image;
    vector<CvScalar> *keypoints;
    CvMat *descriptor;
    int numFeatures;
};

#endif // FEATURE_H
