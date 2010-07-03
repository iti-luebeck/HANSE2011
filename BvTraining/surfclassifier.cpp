#include "surfclassifier.h"
#include "helpers.h"
#include "feature.h"

SURFClassifier::SURFClassifier(double thresh, QList<Mat> objects)
{
    SURFClassifier::thresh = thresh;
    SURFClassifier::objects = objects;
}

void SURFClassifier::classifyOne(int type, vector<KeyPoint> points, Mat features, FoundObject &match)
{
    match.left = INT_MAX;
    match.right = -1;
    match.top = INT_MAX;
    match.bottom = -1;
    match.numMatches = 0;
    match.percentMatches = 0.0;

    if (type < 0 || type >= objects.size())
    {
        return;
    }

    Mat objectFeatures = objects.at(type);

    for (int i = 0; i < (int)points.size(); i++)
    {
        int bestk = -1;
        double best = 1e20;
        double secondBest = 1e20;
        Mat feature = Mat(64, 1, CV_32F, Scalar(0));
        feature = feature + features.col(i);
        for (int j = 0; j < objectFeatures.cols; j++)
        {
            Mat objectFeature = Mat(64, 1, CV_32F, Scalar(0));
            objectFeature = objectFeature + objectFeatures.col(j);
            double acc = norm(feature - objectFeature, NORM_L2);

            if (acc < best)
            {
                secondBest = best;
                best = acc;
                bestk = j;
            }
            else if (acc < secondBest)
            {
                secondBest = acc;
            }
        }

        if (best < thresh * secondBest && bestk >= 0)
        {
            if (points[i].pt.x < match.left)
            {
                match.left = points[i].pt.x;
            }
            if (points[i].pt.x > match.right)
            {
                match.right = points[i].pt.x;
            }
            if (points[i].pt.y < match.top)
            {
                match.top = points[i].pt.y;
            }
            if (points[i].pt.y > match.bottom)
            {
                match.bottom = points[i].pt.y;
            }
            match.numMatches++;
        }
    }

    match.percentMatches = 100.0 * ((double)match.numMatches) / ((double)features.cols);
}

void SURFClassifier::classify(Mat &image, QList<FoundObject> &matches)
{
    Mat imageGray;
    vector<CvScalar> keyPoints;
    vector<KeyPoint> keyPointsCV;
    Feature f;

//  imshow("Image", frame);
//  waitKey();

//  Helpers::convertRGB2Gray(frame, frameGray);
    cvtColor( image, imageGray, CV_RGB2GRAY );
//  imshow("Image", frameGray);
//  waitKey();
    f.findFeatures( new IplImage( imageGray ), keyPoints );
    f.wait();
    CvMat *d = f.getDescriptor();

    Mat features;
    if ( d != NULL )
    {
        for (int i = 0; i < keyPoints.size(); i++)
        {
            keyPointsCV.push_back( KeyPoint( keyPoints[i].val[0], keyPoints[i].val[1], 10, 1, 1, 1, 1 ) );
        }
        features = Mat( d );
        features = features.t();
    }

    matches.clear();
    for (int i = 0; i < objects.size(); i++)
    {
        FoundObject fo;
        classifyOne(i, keyPointsCV, features, fo);
        matches.append(fo);
    }
}

void SURFClassifier::setObjects(QList<Mat>objects)
{
    SURFClassifier::objects = objects;
}

void SURFClassifier::setThresh(double thresh)
{
    SURFClassifier::thresh = thresh;
}

double SURFClassifier::getThresh()
{
    return thresh;
}
