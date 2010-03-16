#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include "cxcore.h"
#include "SVMClassifier.h"

using namespace cv;

class ImageProcessor
{
public:
    ImageProcessor();

    void threshold(Mat img, Mat &thresh);
    void features(Mat &thresh, Mat &features, SVMClassifier &svm);
    void features(Mat &thresh, Mat &features, Mat &classes);
};

#endif // IMAGEPROCESSOR_H
